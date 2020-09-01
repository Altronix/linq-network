#ifndef IO_H
#define IO_H

#include "common.h"
#include "containers.h"
#include "libusb-1.0/libusb.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef const char cchar;
    struct io_s;

    typedef struct io_s
    {
        node_s base;
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
