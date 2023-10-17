#ifndef IO_H
#define IO_H

#include "common/containers.h"
#include "libusb-1.0/libusb.h"
#include "netw.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef const char cchar;
    struct io_s;

    typedef struct
    {
        void (*err)(node_s* n, void*, int err);
    } io_callbacks_s;

    typedef struct io_s
    {
        node_s base;
        io_callbacks_s* callbacks;
        void* ctx;
        libusb_device_handle* handle;
        libusb_device* device;
        struct libusb_device_descriptor desc_dev;
        struct libusb_config_descriptor* desc_cfg;
    } io_s;

    void io_free(io_s** io_p);
    io_s* io_init(libusb_device* d, struct libusb_device_descriptor);

#ifdef __cplusplus
}
#endif
#endif /* IO_H */
