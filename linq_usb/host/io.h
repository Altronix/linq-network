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
        io_ops_s ops;
    } io_s;

    io_s* io_m5_init(libusb_device_handle* handle);

#ifdef __cplusplus
}
#endif
#endif /* IO_H */
