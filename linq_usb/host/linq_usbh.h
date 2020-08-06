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
#include "io.h"
#include "libusb-1.0/libusb.h"

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

    typedef struct descriptors_s
    {
        struct libusb_device_descriptor device;
        struct config
        {
            struct libusb_config_descriptor* config;
            struct interface
            {
                struct libusb_interface_descriptor interface;
                struct libusb_endpoint_descriptor ep[USBH_CONFIG_MAX_ENDPOINT];
            } interface[USBH_CONFIG_MAX_INTERFACE];
        } config[1];
    } descriptors_s;

    typedef struct device_s
    {
        libusb_device_handle* handle;
        libusb_device* device;
        descriptors_s descriptors;
        unsigned char manufacturer[64];
        unsigned char product[64];
        unsigned char serial[64];
        io_s* io;
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
    LINQ_USB_EXPORT int linq_usbh_send_http_request(
        linq_usbh_s* usb,
        const char* serial,
        const char* meth,
        const char* path,
        const char* data,
        ...);
#ifdef __cplusplus
}
#endif
#endif

