#include "wire_event.h"
#include "linq_usbd.h"
#include "log.h"

#define LOG_ERROR_WIRE "(USB) error wire [%d]"
#define LOG_DEBUG_WIRE "(USB device wire [%.*s]"

static void
wire_recv(wire_s* wire, linq_usbd_s* usb, uint8_t* b, uint32_t l)
{
    log_debug(LOG_DEBUG_WIRE, l, b);
}

static void
wire_want_write(wire_s* wire, linq_usbd_s* usb, uint8_t* b, uint32_t l)
{
    log_debug(LOG_DEBUG_WIRE, l, b);
    memcpy(usb->outgoing, b, l);
    int ret = sys_write(usb->io, usb->outgoing, l);
    ((void)ret);
}

static void
wire_error(wire_s* wire, void* ctx, int e)
{
    log_error(LOG_ERROR_WIRE, -1);
}

void
wire_event(wire_s* wire, void* ctx, E_WIRE_EVENT e, ...)
{
#define parse_args(b, l, e, list)                                              \
    va_start(list, e);                                                         \
    b = va_arg(list, uint8_t*);                                                \
    l = va_arg(list, int);                                                     \
    va_end(list);
    va_list list;
    uint8_t* b;
    int l;
    linq_usbd_s* usb = ctx;
    if (WIRE_EVENT_ERROR == e) {
        wire_error(wire, usb, -1);
    } else if (WIRE_EVENT_RECV == e) {
        parse_args(b, l, e, list);
        wire_recv(wire, usb, b, l);
    } else if (WIRE_EVENT_WANT_WRITE == e) {
        parse_args(b, l, e, list);
        wire_want_write(wire, usb, b, l);
    }
#undef parse_args
}
