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
        USB_EVENTS_RECV = 0
    } E_USB_EVENTS;

    typedef struct linq_usbd_s
    {
        sys_file* io;
        struct linq_usbd_callbacks_s
        {
            void (*event)(
                struct linq_usbd_s*,
                E_USB_EVENTS,
                const char* b,
                uint32_t l,
                void* ctx);
        } * callbacks;
        void* ctx;
        char incoming[LINQ_USB_BUFFER_INCOMING_SIZE];
        char outgoing[LINQ_USB_BUFFER_OUTGOING_SIZE];
    } linq_usbd_s;

    LINQ_USB_EXPORT int
    linq_usbd_init(linq_usbd_s* usb, struct linq_usbd_callbacks_s*, void* ctx);
    LINQ_USB_EXPORT void linq_usbd_free(linq_usbd_s* usb);
    LINQ_USB_EXPORT int linq_usbd_poll(linq_usbd_s* usb);
    LINQ_USB_EXPORT int linq_usbd_write(linq_usbd_s*, const char*, ...);

#ifdef __cplusplus
}
#endif
#endif

