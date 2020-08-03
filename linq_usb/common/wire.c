#include "wire.h"
#include "json.h"
#include "rlp.h"
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

int
wire_print_buffer(
    wire_s* wire,
    uint8_t* buffer,
    uint32_t* sz,
    const char* meth,
    const char* path,
    const char* data)
{
    return wire_print(wire, &buffer, sz, meth, path, data);
}

int
wire_print(
    wire_s* wire,
    uint8_t** buffer_p,
    uint32_t* l,
    const char* meth,
    const char* path,
    const char* data)
{
    rlp* r;
    uint32_t sz = *l;
    int err;
    if ((r = rlp_list()) &&       //
        !rlp_push_u8(r, 0) &&     // vers
        !rlp_push_u8(r, 0) &&     // type
        !rlp_push_str(r, meth) && // meth
        !rlp_push_str(r, path) && // path
        !rlp_push_str(r, data)    // data
    ) {
        if (!(*buffer_p)) (*l = sz = rlp_print_size(r), *buffer_p = malloc(sz));
        err = rlp_print(r, *buffer_p, l);
        rlp_free(&r);
        return err;
    } else {
        return -1;
    }
}
