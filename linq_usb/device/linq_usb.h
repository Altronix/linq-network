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
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct linq_usb_callbacks_s
    {
    } linq_usb_callbacks_s;

    typedef struct linq_usb_s
    {
        sys_file* io;
        linq_usb_callbacks_s* callbacks;
        char incoming[LINQ_USB_BUFFER_INCOMING_SIZE];
        char outgoing[LINQ_USB_BUFFER_OUTGOING_SIZE];
    } linq_usb_s;

    LINQ_USB_EXPORT int linq_usb_init(linq_usb_s* usb, linq_usb_callbacks_s*);
    LINQ_USB_EXPORT void linq_usb_free(linq_usb_s* usb);

#ifdef __cplusplus
}
#endif
#endif

