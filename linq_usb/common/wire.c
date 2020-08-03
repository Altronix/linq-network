#include "wire.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

void
wire_init(wire_s* wire, wire_event_fn cb, void* ctx)
{
    memset(wire, 0, sizeof(wire_s));
    wire->state = WIRE_STATE_IDLE;
    wire->cb = cb;
    wire->ctx = ctx;
}

void
wire_free(wire_s* wire)
{
    memset(wire, 0, sizeof(wire_s));
}

void
wire_parse(wire_s* wire, const uint8_t* bytes, uint32_t l)
{
    uint8_t sz;
    if (!(bytes && l >= 3)) return;
    if ((*bytes++ == '\x00')) {
        if ((sz = *bytes++) < 255) {
            (*wire->cb)(wire, wire->ctx, WIRE_EVENT_RECV, bytes, sz);
        } else {
        }
    } else {
        (*wire->cb)(wire, wire->ctx, WIRE_EVENT_ERROR, "version not supported");
    }
}

int
wire_write(wire_s* wire, const char* fmt, ...)
{
    char bytes[254 + 2]; // max size of a short packet
    va_list list;
    va_start(list, fmt);
    int l = vsnprintf(bytes + 2, sizeof(bytes) - 2, fmt, list);
    va_end(list);
    bytes[0] = '\x00';
    bytes[1] = l;
    (*wire->cb)(wire, wire->ctx, WIRE_EVENT_WANT_WRITE, bytes, l + 2);
    return 0;
}
