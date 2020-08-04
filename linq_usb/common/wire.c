#include "wire.h"
#include "json.h"
#include "rlp.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

int
wire_print_buffer(
    uint8_t* buffer,
    uint32_t* sz,
    const char* meth,
    const char* path,
    const char* data)
{
    return wire_print(&buffer, sz, meth, path, data);
}

int
wire_print(
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

void
wire_parser_init(wire_parser_s* wire)
{
    memset(wire, 0, sizeof(wire_parser_s));
}

void
wire_parser_free(wire_parser_s* wire)
{
    if (wire->rlp) rlp_free(&wire->rlp);
}

int
wire_parse(wire_parser_s* wire, const uint8_t* bytes, uint32_t l)
{
    if (wire->rlp) rlp_free(&wire->rlp);
    wire->rlp = rlp_parse(bytes, l);
    return wire->rlp ? 0 : -1;
}

#define make_wire_parser_read_x(id, type, name, idx)                           \
    type wire_parser_read_##id(wire_parser_s* wire)                            \
    {                                                                          \
        const rlp* member = rlp_at(wire->rlp, idx);                            \
        assert(member);                                                        \
        return rlp_as_##name(member);                                          \
    }

make_wire_parser_read_x(vers, uint8_t, u8, 0);
make_wire_parser_read_x(type, uint8_t, u8, 1);
make_wire_parser_read_x(meth, const char*, str, 2);
make_wire_parser_read_x(path, const char*, str, 3);
make_wire_parser_read_x(data, const char*, str, 4);
