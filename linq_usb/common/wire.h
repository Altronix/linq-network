#ifndef WIRE_H
#define WIRE_H

#include "json.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wire_json_s
    {
        json_value vers;
        json_value type;
        json_value meth;
        json_value path;
        json_value data;
    } wire_event_data_s;

    typedef enum E_WIRE_EVENT
    {
        WIRE_EVENT_START,
        WIRE_EVENT_RECV,
        WIRE_EVENT_WANT_WRITE,
        WIRE_EVENT_ERROR
    } E_WIRE_EVENT;

    struct wire_s;
    typedef void (*wire_event_fn)(
        struct wire_s*,
        E_WIRE_EVENT,
        const uint8_t* b,
        uint32_t l,
        wire_event_data_s* data,
        void* ctx);
    typedef struct wire_s
    {
        wire_event_fn cb;
        void* ctx;
    } wire_s;

    void wire_init(wire_s* wire, wire_event_fn, void*);
    void wire_free(wire_s* wire);
    void wire_parse(wire_s*, const uint8_t*, uint32_t);
    int wire_write(wire_s* wire, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif
