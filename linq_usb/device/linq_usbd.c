#include "linq_usbd.h"
#include "log.h"
#include "wire.h"

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
    return sys_read_buffer(usb->io, (char*)usb->incoming, &sz);
}

int
linq_usbd_init(linq_usbd_s* usb)
{
    memset(usb, 0, sizeof(linq_usbd_s));
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
linq_usbd_poll(linq_usbd_s* usb, usbd_event_fn fn, void* ctx)
{
    int len = usb_read(usb);
    if (len > 0) {
        wire_parser_s wire;
        wire_parser_init(&wire);
        len = wire_parse(&wire, usb->incoming, len);
        if (len == 0 && (wire_count(&wire) > 3)) {
            fn(usb,
               ctx,
               USB_EVENTS_TYPE_HTTP,
               wire_parser_read_meth(&wire),
               wire_parser_read_path(&wire),
               wire_count(&wire) > 4 ? wire_parser_read_data(&wire) : NULL);
        } else {
            fn(usb, ctx, USB_EVENTS_ERROR, -1);
        }
        wire_parser_free(&wire);
        memset(usb->incoming, 0, len);
    }
    return len;
}

int
linq_usbd_write_http_request(
    linq_usbd_s* usb,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int ret;
    uint32_t sz = sizeof(usb->outgoing);
    uint8_t* mem = usb->outgoing;
    va_list list;
    va_start(list, data);
    ret = wire_print_http_request_ptr(&mem, &sz, meth, path, data, list);
    va_end(list);
    ret = sys_write(usb->io, (char*)usb->outgoing, sz);
    return ret;
}

int
linq_usbd_write_http_response(
    linq_usbd_s* usb,
    uint16_t code,
    const char* message)
{
    int ret;
    uint32_t sz = sizeof(usb->outgoing);
    ret = wire_print_http_response(usb->outgoing, &sz, code, message);
    if (ret == 0) ret = sys_write(usb->io, (char*)usb->outgoing, sz);
    return ret;
}
