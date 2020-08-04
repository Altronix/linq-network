#ifndef LINQ_USB_H
#define LINQ_USB_H

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

#include "config.h"
#include "wire.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum E_USB_EVENTS
    {
        USB_EVENTS_ERROR = -1,
        USB_EVENTS_TYPE_HTTP = 0
    } E_USB_EVENTS;

    typedef struct linq_usbd_s
    {
        sys_file* io;
        uint8_t incoming[LINQ_USB_BUFFER_INCOMING_SIZE];
        uint8_t outgoing[LINQ_USB_BUFFER_OUTGOING_SIZE];
    } linq_usbd_s;
    typedef void (*usbd_event_fn)(linq_usbd_s*, void*, E_USB_EVENTS, ...);

    LINQ_USB_EXPORT int linq_usbd_init(linq_usbd_s* usb);
    LINQ_USB_EXPORT void linq_usbd_free(linq_usbd_s* usb);
    LINQ_USB_EXPORT int linq_usbd_poll(linq_usbd_s*, usbd_event_fn, void*);
    LINQ_USB_EXPORT int linq_usbd_write_http_request(
        linq_usbd_s* usb,
        const char* meth,
        const char* path,
        const char* data,
        ...);
    LINQ_USB_EXPORT int linq_usbd_write_http_response(
        linq_usbd_s* usb,
        uint16_t code,
        const char* message);

#ifdef __cplusplus
}
#endif
#endif

