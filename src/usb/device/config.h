#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LINQ_USB_CONFIG_IO
#define LINQ_USB_CONFIG_IO "/dev/ttyGS0"
#endif

#ifndef LINQ_USB_BUFFER_INCOMING_SIZE
#define LINQ_USB_BUFFER_INCOMING_SIZE 8192
#endif

#ifndef LINQ_USB_BUFFER_OUTGOING_SIZE
#define LINQ_USB_BUFFER_OUTGOING_SIZE 8192
#endif

#ifdef __cplusplus
}
#endif
#endif

