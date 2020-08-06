#include "io_m5.h"
#include "log.h"
#include "wire.h"

static int
io_m5_vsend_http_request(
    io_s* io_base,
    const char* meth,
    const char* path,
    const char* data,
    va_list list)
{
    io_m5_s* io = (io_m5_s*)io_base;
    int txed, ret;
    uint32_t sz = sizeof(io->out);
    uint8_t* p = io->out;
    ret = wire_print_http_request_ptr(&p, &sz, meth, path, data, list);
    if (ret == 0) {
        ret = libusb_bulk_transfer(io->io.handle, 2, io->out, sz, &txed, 0);
        log_info("(USB) - transfered [%d] bytes", txed);
        if (ret < 0) log_error("(USB) - TX [%s]", libusb_strerror(ret));
    }
    return ret;
}

static int
io_m5_send_http_request(
    io_s* io_base,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int ret;
    va_list list;
    va_start(list, data);
    ret = io_m5_vsend_http_request(io_base, meth, path, data, list);
    va_end(list);
    return ret;
}

io_s*
io_m5_init(libusb_device_handle* handle)
{
    io_m5_s* io = malloc(sizeof(io_m5_s));
    if (io) {
        memset(io, 0, sizeof(io_m5_s));
        io->io.handle = handle;
        io->io.ops.tx = io_m5_send_http_request;
        io->io.ops.vtx = io_m5_vsend_http_request;
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

