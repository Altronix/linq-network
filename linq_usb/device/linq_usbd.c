#include "linq_usbd.h"
#include "log.h"

#define LOG_ERROR_DEVICE "(USB) Failed to open USB device [%s]"
#define LOG_DEBUG_INCOMING "(USB) incoming [%s]"
#define LOG_DEBUG_OUTGOING "(USB) outgoing [%s]"
#define LOG_ERROR_ERROR "(USB) error [%s]"
#define LOG_INFO_INIT "(USB) device open [%s]"
#define LOG_INFO_FREE "(USB) device close [%s]"

static int
usb_read(linq_usbd_s* usb)
{
    uint32_t sz = sizeof(usb->incoming);
    return sys_read_buffer(usb->io, usb->incoming, &sz);
}

int
linq_usbd_init(
    linq_usbd_s* usb,
    struct linq_usbd_callbacks_s* callbacks,
    void* ctx)
{
    memset(usb, 0, sizeof(linq_usbd_s));
    usb->callbacks = callbacks;
    usb->ctx = ctx;
    usb->io = sys_open(LINQ_USB_CONFIG_IO, FILE_MODE_READ_WRITE);
    if (!usb->io) {
        log_error(LOG_ERROR_DEVICE, LINQ_USB_CONFIG_IO);
    } else {
        log_info(LOG_INFO_INIT, LINQ_USB_CONFIG_IO);
    }
    return usb->io ? 0 : -1;
}

void
linq_usbd_free(linq_usbd_s* usb)
{
    if (usb->io) {
        sys_close(&usb->io);
        log_info(LOG_INFO_FREE, LINQ_USB_CONFIG_IO);
    }
    memset(usb, 0, sizeof(linq_usbd_s));
}

int
linq_usbd_poll(linq_usbd_s* usb)
{
    int len = usb_read(usb);
    if (len > 0) {
        usb->callbacks->event(
            usb, USB_EVENTS_RECV, usb->incoming, len, usb->ctx);
    }
    return len;
}
