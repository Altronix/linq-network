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
#include "wire.h"
#include "libusb-1.0/libusb.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct linq_usbh_s
    {
        libusb_context* context;
    } linq_usbh_s;

    LINQ_USB_EXPORT void linq_usbh_init(linq_usbh_s* usb);
    LINQ_USB_EXPORT void linq_usbh_free(linq_usbh_s* usb);
    LINQ_USB_EXPORT int linq_usbh_scan(linq_usbh_s*, char*, uint32_t* l);

#ifdef __cplusplus
}
#endif
#endif

