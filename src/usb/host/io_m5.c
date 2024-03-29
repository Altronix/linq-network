#define _GNU_SOURCE
#include "io_m5.h"
#include "common/log.h"
#include "errno.h"
#include "usb/common/wire.h"
#include <string.h>

#define usb_info(...) log_info("USB", __VA_ARGS__)
#define usb_warn(...) log_warn("USB", __VA_ARGS__)
#define usb_debug(...) log_debug("USB", __VA_ARGS__)
#define usb_trace(...) log_trace("USB", __VA_ARGS__)
#define usb_error(...) log_error("USB", __VA_ARGS__)
#define usb_fatal(...) log_fatal("USB", __VA_ARGS__)

#define IN 1 | LIBUSB_ENDPOINT_IN
#define OUT 2 | LIBUSB_ENDPOINT_OUT

// TODO handle LIBUSB_ERROR_NO_DEVICE (-4) By removing device from device_map
//      handle LIBUSB_ERROR_TIMEOUT (-7)

typedef enum E_IO_STATE
{
    IO_STATE_IDLE,
    IO_STATE_MORE,
    IO_STATE_DONE
} E_IO_STATE;

typedef struct io_m5_packet_s
{
    E_IO_STATE state;
    uint32_t c, l;
    uint8_t* bytes;
} io_m5_packet_s;
LIST_INIT_W_FREE(packet, io_m5_packet_s);

typedef struct io_m5_s
{
    io_s io; // Class Base
    packet_list_s* outgoing;
    packet_list_s* incoming;
    uint8_t out[IO_M5_MAX_OUTGOING];
    uint8_t in[IO_M5_MAX_INCOMING];
} io_m5_s;

// TODO move to common
static E_REQUEST_METHOD
method_from_str(const char* method)
{
    uint32_t l = strlen(method);
    if (l == 3) {
        if (!memcmp(method, "GET", l)) {
            return REQUEST_METHOD_GET;
        } else if (!(memcmp(method, "PUT", l))) {
            return REQUEST_METHOD_POST; // TODO support PUT
        }
    } else if (l == 4 && !memcmp(method, "POST", l)) {
        return REQUEST_METHOD_POST;
    } else if (l == 6 && !memcmp(method, "DELETE", l)) {
        return REQUEST_METHOD_DELETE;
    }
    assert(false);
    return -1; // should never return
}

static E_LINQ_ERROR
map_libusb_error(int err)
{
    E_LINQ_ERROR e = err;
    if (LIBUSB_ERROR_NO_DEVICE == err) {
        e = LINQ_ERROR_404;
    } else if (LIBUSB_ERROR_TIMEOUT == err) {
        e = LINQ_ERROR_504;
    }
    return e;
}

// TODO move to common
static const char*
method_to_str(E_REQUEST_METHOD m)
{
    static const char* put = "PUT";
    static const char* post = "POST";
    static const char* delete = "DELETE";
    static const char* get = "GET";
    if (REQUEST_METHOD_GET == m) {
        return get;
    } else if (REQUEST_METHOD_POST == m) {
        return post;
    } else if (REQUEST_METHOD_DELETE == m) {
        return delete;
    } else {
        return put;
    }
}

static int
io_m5_send_http_request_sync(
    io_s* io_base,
    E_REQUEST_METHOD method,
    const char* path,
    uint32_t plen,
    const char* data,
    uint32_t dlen)
{
    // TODO "transferred (ie txed) may not always equal sz
    // Will have to send in loop since we are allowed to be sync here
    io_m5_s* io = (io_m5_s*)io_base;
    const char* meth = method_to_str(method);
    int txed, ret;
    uint32_t sz = sizeof(io->out);
    uint8_t* p = io->out;
    ret = wire_print_http_request_n(&p, &sz, meth, path, plen, data, dlen);
    if (ret == 0) {
        ret = libusb_bulk_transfer(io->io.handle, OUT, io->out, sz, &txed, 0);
        usb_info("tx [%d/%d] bytes", txed, sz);
        if (!(ret < 0) && !(txed == sz)) usb_error("TODO tx_sync incomplete!");
        if (ret < 0) usb_error("tx [%s]", libusb_strerror(ret));
    }
    return ret;
}

static void
io_m5_poll(node_s* io_base, void* ctx)
{}

static int
io_m5_recv_http_response_sync(
    struct io_s* io_base,
    uint16_t* code,
    char* mesg,
    uint32_t mesg_sz)
{
#define err_exit(code, m1, m2)                                                 \
    do {                                                                       \
        usb_error("rx [%s %s]", m1, m2);                                       \
        goto EXIT;                                                             \
    } while (0)

    io_m5_s* io = (io_m5_s*)io_base;
    int txed, ret;
    uint32_t sz = sizeof(io->in), n = 0;
    uint8_t* ptr = NULL;

    // Read some bytes (exit if error)
    libusb_device_handle* handle = io->io.handle;
    ret = libusb_bulk_transfer(handle, IN, io->in, sz, &txed, 2000);
    if (ret < 0) err_exit(ret, libusb_strerror(ret), strerror(errno));
    usb_info("rx [%d/%d]", txed, sz);
    while (!(ptr = memmem(io->in, txed, "\r\n", 2)) && txed) {
        n = txed;
        ret = libusb_bulk_transfer(handle, IN, &io->in[txed], sz, &txed, 2000);
        if (ret < 0) err_exit(ret, libusb_strerror(ret), strerror(errno));
        usb_info("rx [%d/%d]", txed, sz);
        txed += n;
    }

    // Check advert size must equal received size (received = expect + \n)
    ret = wire_read_sz(&sz, io->in, txed);
    usb_info("rx [%d/%d]", txed, sz);
    if (!(ret == 0)) err_exit(ret, "underflow detected", "");
    if (!(sz <= txed)) err_exit(-1, "size advertise missmatch", "");

    // Arrive here with complete packet ready to parse
    wire_parser_http_response_s r;
    ret = wire_parse_http_response(io->in, txed, &r);
    if (ret == 0) {
        *code = r.code;
        snprintf(mesg, mesg_sz, "%s", r.mesg);
        wire_parser_http_response_free(&r);
    }
EXIT:
    usb_info("rx result [%d]", ret);
    return ret;
#undef err_exit
}

static void
process_io_error(io_s* io, int err, linq_request_complete_fn fn, void* ctx)
{
    const char* e;
    char buff[512];
    e = libusb_strerror(err);
    usb_error("rx err %s", e);
    snprintf(buff, sizeof(buff), "{\"error\": \"%s\",\"code\":%d}", e, err);
    fn(ctx, device_serial(&io->base), map_libusb_error(err), buff);
    if (io->callbacks && io->callbacks->err) {
        io->callbacks->err(&io->base, io->ctx, err);
    }
}

static void
io_m5_send(
    struct node_s* base,
    E_REQUEST_METHOD meth,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_request_complete_fn fn,
    void* ctx)
{
    char m[8192]; // TODO need linked list to of requests and get rid of stack
    const char* e;
    uint16_t code;
    io_s* io = (io_s*)base;
    int err = io_m5_send_http_request_sync(io, meth, path, plen, json, jlen);
    if (!err) {
        err = io_m5_recv_http_response_sync(io, &code, m, sizeof(m));
        if (!err) {
            fn(ctx, device_serial(&io->base), code, m);
        } else {
            process_io_error(io, err, fn, ctx);
        }
    } else {
        process_io_error(io, err, fn, ctx);
    }
}

node_s*
io_m5_init(
    libusb_device* d,
    struct libusb_device_descriptor descriptor,
    io_callbacks_s* callbacks,
    void* ctx)
{
    uint8_t encoding[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 };
    int err;
    io_m5_s* device = malloc(sizeof(io_m5_s));
    if (device) {
        memset(device, 0, sizeof(io_m5_s));
        device->io.callbacks = callbacks;
        device->io.ctx = ctx;
        err = libusb_open(d, &device->io.handle);
        if (err) {
            free(device);
            device = NULL;
        } else {
            device->incoming = packet_list_create();
            device->outgoing = packet_list_create();
            device->io.device = d;
            device->io.desc_dev = descriptor;
            assert(device->incoming && device->outgoing);
            err = libusb_get_config_descriptor(
                device->io.device, 0, &device->io.desc_cfg);
            if (err) { usb_error("cfg [%s]", libusb_strerror(err)); }
            err = libusb_get_string_descriptor_ascii(
                device->io.handle,
                descriptor.iProduct,
                (unsigned char*)device->io.base.type,
                sizeof(device->io.base.type));
            if (err < 0) usb_error("str [%s]", libusb_strerror(err));
            err = libusb_get_string_descriptor_ascii(
                device->io.handle,
                descriptor.iSerialNumber,
                (unsigned char*)device->io.base.serial,
                sizeof(device->io.base.serial));
            if (err < 0) usb_error("str [%s]", libusb_strerror(err));
            for (int i = 0; i < 2; i++) {
                if (libusb_kernel_driver_active(device->io.handle, i)) {
                    usb_info("detatching kernel driver [%d]", i);
                    err = libusb_detach_kernel_driver(device->io.handle, i);
                    if (err) usb_error("[%s]", libusb_strerror(err));
                }
                usb_info("claiming interface [%d]", i);
                err = libusb_claim_interface(device->io.handle, i);
                if (err) usb_error("[%s]", libusb_strerror(err));
            }

            // https://github.com/tytouf/libusb-cdc-example/blob/master/cdc_example.c
            err = libusb_control_transfer(
                device->io.handle, 0x21, 0x22, 0x01 | 0x02, 0, NULL, 0, 0);
            if (err < 0) usb_error("ctr [%s]", libusb_strerror(err));
            err = libusb_control_transfer(
                device->io.handle,
                0x21,
                0x20,
                0,
                0,
                encoding,
                sizeof(encoding),
                0);
            if (err < 0) usb_error("ctr [%s]", libusb_strerror(err));
            device->io.base.free = io_m5_free;
            device->io.base.send = io_m5_send;
            device->io.base.poll = io_m5_poll;
            // device->io.base.poll = // TODO
            // device->io.base.send = // TODO
            // TODO - install io driver per product type
            //      - Optionally ? libusb_set_configuration( ...
            //      - Optionally ? libusb_claim_interface( ...
        }
    }
    return (node_s*)device;
}

void
io_m5_free(node_s** io_p)
{
    io_m5_s* io = *(io_m5_s**)io_p;
    io_p = NULL;
    packet_list_destroy(&io->incoming);
    packet_list_destroy(&io->outgoing);
    if (io->io.desc_cfg) { libusb_free_config_descriptor(io->io.desc_cfg); }
    libusb_close(io->io.handle);
    free(io);
}

