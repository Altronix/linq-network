#include "io_m5.h"
#include "errno.h"
#include "log.h"
#include "wire.h"

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

    // Read some bytes (exit if error)
    libusb_device_handle* handle = io->io.handle;
    ret = libusb_bulk_transfer(handle, IN, io->in, sz, &txed, 2000);
    if (ret < 0) err_exit(ret, libusb_strerror(ret), strerror(errno));

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
io_m5_init(libusb_device_handle* handle)
{
    io_m5_s* io = malloc(sizeof(io_m5_s));
    if (io) {
        memset(io, 0, sizeof(io_m5_s));
        io->io.handle = handle;
        io->io.ops.tx_sync = io_m5_send_http_request_sync;
        io->io.ops.vtx_sync = io_m5_vsend_http_request_sync;
        io->io.ops.rx_sync = io_m5_recv_http_response_sync;
    }
    return (io_s*)io;
}

void
io_m5_free(io_s** io_p)
{
    io_m5_s* io = *(io_m5_s**)io_p;
    io_p = NULL;
    free(io);
}

