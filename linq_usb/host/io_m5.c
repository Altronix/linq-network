#define _GNU_SOURCE
#include "io_m5.h"
#include "errno.h"
#include "log.h"
#include "wire.h"
#include <string.h>

#define IN 1 | LIBUSB_ENDPOINT_IN
#define OUT 2 | LIBUSB_ENDPOINT_OUT

static int
io_m5_vsend_http_request_sync(
    io_s* io_base,
    const char* meth,
    const char* path,
    const char* data,
    va_list list)
{
    // TODO "transferred (ie txed) may not always equal sz
    // Will have to send in loop since we are allowed to be sync here
    io_m5_s* io = (io_m5_s*)io_base;
    int txed, ret;
    uint32_t sz = sizeof(io->out);
    uint8_t* p = io->out;
    ret = wire_print_http_request_ptr(&p, &sz, meth, path, data, list);
    if (ret == 0) {
        ret = libusb_bulk_transfer(io->io.handle, OUT, io->out, sz, &txed, 0);
        log_info("(USB) - tx [%d/%d] bytes", txed, sz);
        if (!(ret < 0) && !(txed == sz)) log_error("TODO tx_sync incomplete!");
        if (ret < 0) log_error("(USB) - TX [%s]", libusb_strerror(ret));
    }
    return ret;
}

static int
io_m5_send_http_request_sync(
    io_s* io_base,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int ret;
    va_list list;
    va_start(list, data);
    ret = io_m5_vsend_http_request_sync(io_base, meth, path, data, list);
    va_end(list);
    return ret;
}

static int
io_m5_recv_http_response_sync(
    struct io_s* io_base,
    uint16_t* code,
    char* mesg,
    uint32_t mesg_sz)
{
#define err_exit(code, m1, m2)                                                 \
    do {                                                                       \
        log_error("(USB) - rx [%s %s]", m1, m2);                               \
        goto EXIT;                                                             \
    } while (0)

    io_m5_s* io = (io_m5_s*)io_base;
    int txed, ret;
    uint32_t sz = sizeof(io->in), n;
    uint8_t* ptr = NULL;

    // Read some bytes (exit if error)
    libusb_device_handle* handle = io->io.handle;
    ret = libusb_bulk_transfer(handle, IN, io->in, sz, &txed, 2000);
    if (ret < 0) err_exit(ret, libusb_strerror(ret), strerror(errno));
    log_info("(USB) - rx [%d/%d]", txed, sz);
    if (!(ptr = memmem(io->in, txed, "\r\n", 2))) {
        n = txed;
        ret = libusb_bulk_transfer(handle, IN, &io->in[txed], sz, &txed, 2000);
        if (ret < 0) err_exit(ret, libusb_strerror(ret), strerror(errno));
        log_info("(USB) - rx [%d/%d]", txed, sz);
        txed += n;
    }

    // Check advert size must equal received size (received = expect + \n)
    ret = wire_read_sz(&sz, io->in, txed);
    log_info("(USB) - rx [%d/%d]", txed, sz);
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
    log_info("(USB) - rx result [%d]", ret);
    return ret;
#undef err_exit
}

io_s*
io_m5_init(libusb_device* d, struct libusb_device_descriptor descriptor)
{
    uint8_t encoding[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 };
    int err;
    io_m5_s* device = malloc(sizeof(io_m5_s));
    if (device) {
        memset(device, 0, sizeof(io_m5_s));
        err = libusb_open(d, &device->io.handle);
        if (err) {
            free(device);
            device = NULL;
        } else {
            device->io.device = d;
            device->io.desc_dev = descriptor;
            err = libusb_get_config_descriptor(
                device->io.device, 0, &device->io.desc_cfg);
            if (err) { log_error("(USB) - cfg [%s]", libusb_strerror(err)); }
            err = libusb_get_string_descriptor_ascii(
                device->io.handle,
                descriptor.iManufacturer,
                device->io.manufacturer,
                sizeof(device->io.manufacturer));
            if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
            err = libusb_get_string_descriptor_ascii(
                device->io.handle,
                descriptor.iProduct,
                device->io.product,
                sizeof(device->io.product));
            if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
            err = libusb_get_string_descriptor_ascii(
                device->io.handle,
                descriptor.iSerialNumber,
                device->io.serial,
                sizeof(device->io.serial));
            if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
            for (int i = 0; i < 2; i++) {
                if (libusb_kernel_driver_active(device->io.handle, i)) {
                    log_info("(USB) - detatching kernel driver [%d]", i);
                    err = libusb_detach_kernel_driver(device->io.handle, i);
                    if (err) log_error("(USB) - [%s]", libusb_strerror(err));
                }
                log_info("(USB) - claiming interface [%d]", i);
                err = libusb_claim_interface(device->io.handle, i);
                if (err) log_error("(USB) - [%s]", libusb_strerror(err));
            }
            // https://github.com/tytouf/libusb-cdc-example/blob/master/cdc_example.c
            err = libusb_control_transfer(
                device->io.handle, 0x21, 0x22, 0x01 | 0x02, 0, NULL, 0, 0);
            if (err < 0) log_error("USB) - ctr [%s]", libusb_strerror(err));
            err = libusb_control_transfer(
                device->io.handle,
                0x21,
                0x20,
                0,
                0,
                encoding,
                sizeof(encoding),
                0);
            if (err < 0) log_error("USB) - ctr [%s]", libusb_strerror(err));
            device->io.ops.tx_sync = io_m5_send_http_request_sync;
            device->io.ops.vtx_sync = io_m5_vsend_http_request_sync;
            device->io.ops.rx_sync = io_m5_recv_http_response_sync;
            // TODO - install io driver per product type
            //      - Optionally ? libusb_set_configuration( ...
            //      - Optionally ? libusb_claim_interface( ...
        }
    }
    return (io_s*)device;
}

void
io_m5_free(io_s** io_p)
{
    io_m5_s* io = *(io_m5_s**)io_p;
    io_p = NULL;
    free(io);
}

