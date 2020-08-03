#include "linq_usbd.h"
#include "log.h"
#include "wire.h"
#include "wire_event.h"

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

static int
usb_write(linq_usbd_s* usb, void* b, uint32_t l)
{
    memcpy(usb->outgoing, b, l);
    return sys_write(usb->io, usb->outgoing, l);
}

int
linq_usbd_init(
    linq_usbd_s* usb,
    struct linq_usbd_callbacks_s* callbacks,
    void* ctx)
{
    memset(usb, 0, sizeof(linq_usbd_s));
    // wire_init(&usb->wire, wire_event, usb);
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
    wire_free(&usb->wire);
    memset(usb, 0, sizeof(linq_usbd_s));
}

int
linq_usbd_poll(linq_usbd_s* usb)
{
    int len = usb_read(usb);
    if (len > 0) {
        // wire_parse(&usb->wire, (const uint8_t*)usb->incoming, len);
        memset(usb->incoming, 0, len);
    }
    return len;
}

int
linq_usbd_write(linq_usbd_s* usb, const char* fmt, ...)
{
    // TODO must go through wire layer to wrap payload with packet
    int ret;
    va_list list;
    va_start(list, fmt);
    ret = sys_vfprintf(usb->io, fmt, list);
    va_end(list);
    return ret;
}
