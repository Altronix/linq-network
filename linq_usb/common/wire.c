#include "wire.h"
#include "json.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

void
wire_init(wire_s* wire, wire_event_fn cb, void* ctx)
{
    memset(wire, 0, sizeof(wire_s));
    wire->cb = cb;
    wire->ctx = ctx;
}

void
wire_free(wire_s* wire)
{
    memset(wire, 0, sizeof(wire_s));
}

void
wire_parse(wire_s* wire, const uint8_t* b, uint32_t l)
{
    json_parser p;
    wire_event_data_s ev_data;
    jsontok t[48];
    uint8_t sz;
    int err = 0;
    if (!(b && l >= 3)) return;
    if ((*b++ == '\x00')) {
        if ((sz = *b++) < 255) {
            json_init(&p);
            err = json_parse(&p, (const char*)b, sz, t, 48);
            if (err > 0) {
                ev_data.vers = json_delve_value((const char*)b, t, ".vers");
                ev_data.type = json_delve_value((const char*)b, t, ".type");
                ev_data.meth = json_delve_value((const char*)b, t, ".meth");
                ev_data.path = json_delve_value((const char*)b, t, ".path");
                ev_data.data = json_delve_value((const char*)b, t, ".data");
                (*wire->cb)(wire, WIRE_EVENT_RECV, b, l, &ev_data, wire->ctx);
            } else {
                // JSON PARSER ERROR
            }
        } else {
            // LONG DATA FORMAT
        }
    } else {
        // INCOMPATIBLE VERSION
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
    // (*wire->cb)(wire, wire->ctx, WIRE_EVENT_WANT_WRITE, bytes, l + 2);
    return 0;
}
