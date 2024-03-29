#ifndef LINQ_USBH_H
#define LINQ_USBH_H

#include "netw.h"

#include "common/containers.h"
#include "common/device.h"
#include "common/sys/sys.h"
#include "libusb-1.0/libusb.h"
#include "usb/host/io.h"

#ifndef USBH_CONFIG_MAX_INTERFACE
#define USBH_CONFIG_MAX_INTERFACE 2
#endif

#ifndef USBH_CONFIG_MAX_ENDPOINT
#define USBH_CONFIG_MAX_ENDPOINT 2
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct usbh_s
    {
        libusb_context* context;
        device_map_s** devices_p;
    } usbh_s;

    LINQ_EXPORT void usbh_init(usbh_s* usb, device_map_s** devices_p);
    LINQ_EXPORT void usbh_free(usbh_s* usb);
    LINQ_EXPORT int usbh_poll(usbh_s* usb, uint32_t ms);
    LINQ_EXPORT int usbh_scan(usbh_s*, uint16_t, uint16_t);
    LINQ_EXPORT uint32_t usbh_device_count(usbh_s* usb);
    LINQ_EXPORT int usbh_print_devices(usbh_s*, char*, uint32_t);
#ifdef __cplusplus
}
#endif
#endif

