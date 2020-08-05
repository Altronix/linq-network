#ifndef LINQ_USBH_H
#define LINQ_USBH_H

// clang-format off
#if defined _WIN32
#  if defined LINQ_USB_STATIC
#    define LINQ_USB_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_USB_EXPORT __declspec(dllexport)
#  else
#    define LINQ_USB_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_USB_EXPORT
#endif
// clang-format on

#include "sys.h"
#include "containers.h"
#include "wire.h"
#include "libusb-1.0/libusb.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device_s
    {
        libusb_device_handle* handle;
        libusb_device* device;
        struct libusb_device_descriptor descriptor;
        unsigned char manufacturer[64];
        unsigned char product[64];
        unsigned char serial[64];
    } device_s;
    MAP_INIT_H(device, device_s);

    typedef struct linq_usbh_s
    {
        libusb_context* context;
        device_map_s* devices;
    } linq_usbh_s;

    LINQ_USB_EXPORT void linq_usbh_init(linq_usbh_s* usb);
    LINQ_USB_EXPORT void linq_usbh_free(linq_usbh_s* usb);
    LINQ_USB_EXPORT int linq_usbh_scan(linq_usbh_s*, uint16_t, uint16_t);
    LINQ_USB_EXPORT uint32_t linq_usbh_device_count(linq_usbh_s* usb);
    LINQ_USB_EXPORT int linq_usbh_print_devices(linq_usbh_s*, char*, uint32_t);

#ifdef __cplusplus
}
#endif
#endif

