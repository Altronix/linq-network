#ifndef IO_H
#define IO_H

#include "libusb-1.0/libusb.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef const char cchar;
    struct io_s;
    typedef struct io_ops_s
    {
        int (*tx)(struct io_s*, cchar*, cchar*, cchar*, ...);
        int (*tx_sync)(struct io_s*, cchar*, cchar*, cchar*, ...);
        int (*vtx)(struct io_s*, cchar*, cchar*, cchar*, va_list);
        int (*vtx_sync)(struct io_s*, cchar*, cchar*, cchar*, va_list);
        int (*rx)(struct io_s*, uint16_t*, char*, uint32_t);
        int (*rx_sync)(struct io_s*, uint16_t*, char*, uint32_t);
    } io_ops_s;

    typedef struct
    {
        uint32_t l;
        uint8_t bytes[];
    } io_packet_s;

    typedef struct io_s
    {
        libusb_device_handle* handle;
        libusb_device* device;
        struct libusb_device_descriptor desc_dev;
        struct libusb_config_descriptor* desc_cfg;
        unsigned char manufacturer[64];
        unsigned char product[64];
        unsigned char serial[64];
        io_ops_s ops;
    } io_s;

    void io_free(io_s** io_p);
    io_s* io_init(libusb_device* d, struct libusb_device_descriptor);

#ifdef __cplusplus
}
#endif
#endif /* IO_H */
