#include "io_m5.h"
#include "log.h"
#include "wire.h"

#define IN 1
#define OUT 2

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
        log_info("(USB) - transfered [%d/%d] bytes", txed, sz);
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
    uint32_t sz)
{
    // TODO
    // Read a few bytes to get the RLP size (+ readin newline)
    // Then read in the rest... (make sure not to not include newlines in size
    // calculations
    // [00,7f] single byte between 0-127 {byte}
    // [80,b7] 0-55 byte item            {80+size of data}{data...}
    // [b8,bf] 55+ byte item             {b8+size of length}{length}{data...}
    // [c0,f7] 0-55 byte list            {c0+size of nested items}{data...}
    // [f8,ff] 55+ byte list             {c0+size of length}{length}{data...}
    io_m5_s* io = (io_m5_s*)io_base;
    int txed, ret;
    uint32_t l = sizeof(io->in);
    ret = libusb_bulk_transfer(io->io.handle, IN, io->in, l, &txed, 0);
    if (ret == 0) {
        log_info("(USB) - transfered [%d] bytes", txed);
        wire_parser_http_response_s r;
        wire_parse_http_response(io->in, txed, &r);
        *code = r.code;
        snprintf(mesg, sz, "%s", r.mesg);
        wire_parser_http_response_free(&r);
    } else {
        log_error("(USB) - rx [%s]", libusb_strerror(ret));
    }
    return ret;
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

