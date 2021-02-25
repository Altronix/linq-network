#include "usbd.h"
#include "errno.h"
#include "log.h"
#include "wire.h"

#define usb_info(...) log_info("USB", __VA_ARGS__)
#define usb_warn(...) log_warn("USB", __VA_ARGS__)
#define usb_debug(...) log_debug("USB", __VA_ARGS__)
#define usb_trace(...) log_trace("USB", __VA_ARGS__)
#define usb_error(...) log_error("USB", __VA_ARGS__)
#define usb_fatal(...) log_fatal("USB", __VA_ARGS__)

#define LOG_ERROR_DEVICE "Failed to open USB device [%s]"
#define LOG_DEBUG_INCOMING "incoming [%s]"
#define LOG_DEBUG_OUTGOING "outgoing [%s]"
#define LOG_ERROR_ERROR "error [%s]"
#define LOG_INFO_INIT "device open [%s]"
#define LOG_INFO_FREE "device close [%s]"

static int
usb_read(usbd_s* usb)
{
    uint32_t sz = sizeof(usb->incoming);
    return sys_read_buffer(usb->io, (char*)usb->incoming, &sz);
}

int
usbd_init(usbd_s* usb)
{
    memset(usb, 0, sizeof(usbd_s));
    const char* p = LINQ_USB_CONFIG_IO;
    usb->io = sys_open(p, FILE_MODE_READ_WRITE, FILE_NON_BLOCKING);
    if (!usb->io) {
        usb_error(LOG_ERROR_DEVICE, LINQ_USB_CONFIG_IO);
    } else {
        usb_info(LOG_INFO_INIT, LINQ_USB_CONFIG_IO);
    }
    return usb->io ? 0 : -1;
}

void
usbd_free(usbd_s* usb)
{
    if (usb->io) {
        sys_close(&usb->io);
        usb_info(LOG_INFO_FREE, LINQ_USB_CONFIG_IO);
    }
    memset(usb, 0, sizeof(usbd_s));
}

int
usbd_poll(usbd_s* usb, usbd_event_fn fn, void* ctx)
{
    int len = usb_read(usb), count;
    if (len > 0) {
        usb_info("recv [%d]", len);
        wire_parser_http_request_s request;
        int rc;
        rc = wire_parse_http_request(usb->incoming, len, &request);
        if (rc == 0) {
            fn(usb,
               ctx,
               USB_EVENTS_TYPE_HTTP,
               request.meth,
               request.path,
               request.data ? request.data : NULL);
            wire_parser_http_request_free(&request);
        } else {
            fn(usb, ctx, USB_EVENTS_ERROR, -1);
        }
        memset(usb->incoming, 0, len);
    } else if (len < 0) {
        usb_error("recv [%s]", strerror(errno));
    }
    return len;
}

int
usbd_write_http_request(
    usbd_s* usb,
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
usbd_write_http_response(usbd_s* usb, uint16_t code, const char* message)
{
    int ret;
    uint32_t sz = sizeof(usb->outgoing);
    ret = wire_print_http_response(usb->outgoing, &sz, code, message);
    if (ret == 0) ret = sys_write(usb->io, (char*)usb->outgoing, sz);
    return ret;
}
