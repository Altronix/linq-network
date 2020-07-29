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

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#endif

