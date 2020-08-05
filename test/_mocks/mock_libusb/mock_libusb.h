#ifndef MOCK_LIBUSB_H
#define MOCK_LIBUSB_H

#include "libusb-1.0/libusb.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void spy_libusb_init();
    void spy_libusb_free();
    void spy_libusb_push_device(struct libusb_device_descriptor* desc);

#ifdef __cplusplus
}
#endif
#endif /* MOCK_LIBUSB_H */
