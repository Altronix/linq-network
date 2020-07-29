#include "linq_usb.h"
#include "log.h"

#define LOG_ERROR_DEVICE "(USB) Failed to open USB device [%s]"
#define LOG_DEBUG_INCOMING "(USB) incoming [%s]"
#define LOG_DEBUG_OUTGOING "(USB) outgoing [%s]"
#define LOG_ERROR_ERROR "(USB) error [%s]"

int
linq_usb_init(linq_usb_s* usb, linq_usb_callbacks_s* callbacks)
{
    memset(usb, 0, sizeof(linq_usb_s));
    usb->callbacks = callbacks;
    usb->io = sys_open(LINQ_USB_CONFIG_IO, FILE_MODE_READ_WRITE);
    if (!usb->io) log_error(LOG_ERROR_DEVICE, LINQ_USB_CONFIG_IO);
    return usb->io ? 0 : -1;
}

void
linq_usb_free(linq_usb_s* usb)
{
    if (usb->io) sys_close(&usb->io);
    memset(usb, 0, sizeof(linq_usb_s));
}
