#ifndef LINQ_USBD_H
#define LINQ_USBD_H

// clang-format off
#if defined _WIN32
#  if defined LINQ_STATIC
#    define LINQ_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_EXPORT __declspec(dllexport)
#  else
#    define LINQ_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_EXPORT
#endif
// clang-format on

#include "config.h"
#include "usb/common/wire.h"
#include "common/sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum E_USB_EVENTS
    {
        USB_EVENTS_ERROR = -1,
        USB_EVENTS_TYPE_HTTP = 0
    } E_USB_EVENTS;

    typedef struct usbd_s
    {
        sys_file* io;
        uint8_t incoming[LINQ_USB_BUFFER_INCOMING_SIZE];
        uint8_t outgoing[LINQ_USB_BUFFER_OUTGOING_SIZE];
    } usbd_s;
    typedef void (*usbd_event_fn)(usbd_s*, void*, E_USB_EVENTS, ...);

    LINQ_EXPORT int usbd_init(usbd_s* usb);
    LINQ_EXPORT void usbd_free(usbd_s* usb);
    LINQ_EXPORT int usbd_poll(usbd_s*, usbd_event_fn, void*);
    LINQ_EXPORT int usbd_write_http_request(
        usbd_s* usb,
        const char* meth,
        const char* path,
        const char* data,
        ...);
    LINQ_EXPORT int
    usbd_write_http_response(usbd_s* usb, uint16_t code, const char* message);

#ifdef __cplusplus
}
#endif
#endif

