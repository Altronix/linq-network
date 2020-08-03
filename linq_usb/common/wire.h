#ifndef WIRE_H
#define WIRE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

    typedef enum E_WIRE_STATE
    {
        WIRE_STATE_IDLE,
        WIRE_STATE_START
    } E_WIRE_STATE;

    typedef enum E_WIRE_EVENT
    {
        WIRE_EVENT_START,
        WIRE_EVENT_RECV,
        WIRE_EVENT_WANT_WRITE,
        WIRE_EVENT_ERROR
    } E_WIRE_EVENT;

    struct wire_s;
    typedef void (*wire_event_fn)(struct wire_s*, void*, E_WIRE_EVENT, ...);
    typedef struct wire_s
    {
        E_WIRE_STATE state;
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
