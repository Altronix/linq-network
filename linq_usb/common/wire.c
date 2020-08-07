#include "wire.h"
#include "json.h"
#include "rlp.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#define make_wire_parser_read_x(id, type, name, idx)                           \
    static type wire_parser_read_##id(wire_parser_s* wire)                     \
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
make_wire_parser_read_x(code, uint16_t, u16, 2);
make_wire_parser_read_x(mesg, const char*, str, 3);

static void
add_terminator(uint8_t* spot, uint32_t* l, uint32_t sz)
{
#if HAS_TERMINATE == TRUE
    if (*l < sz) spot[*l] = WIRE_TERMINATE;
    *l = *l + 1;
#endif
}

int
rlp_vpush_str(rlp* parent, const char* data, va_list list)
{
    // TODO move into rlp?
    char* mem;
    int ret;
    va_list dupe;
    va_copy(dupe, list);
    ret = vsnprintf(NULL, 0, data, dupe);
    va_end(dupe);
    mem = malloc(ret + 1);
    assert(mem);
    vsnprintf(mem, ret + 1, data, list);
    ret = rlp_push_str(parent, mem);
    free(mem);
    return ret;
}

int
wire_print_http_request(
    uint8_t* buffer,
    uint32_t* sz,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int ret;
    va_list list;
    va_start(list, data);
    ret = wire_print_http_request_ptr(&buffer, sz, meth, path, data, list);
    va_end(list);
    return ret;
}

int
wire_print_http_request_alloc(
    uint8_t** buffer_p,
    uint32_t* l,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int ret;
    assert(buffer_p);
    assert(*buffer_p == NULL);
    va_list list;
    va_start(list, data);
    ret = wire_print_http_request_ptr(buffer_p, l, meth, path, data, list);
    va_end(list);
    return ret;
}

int
wire_print_http_request_ptr(
    uint8_t** buffer_p,
    uint32_t* l,
    const char* meth,
    const char* path,
    const char* data,
    va_list list)
{
    rlp* r;
    uint32_t sz = *l;
    int err;
    if ((r = rlp_list()) &&                           //
        !rlp_push_u8(r, 0) &&                         // vers
        !rlp_push_u8(r, 0) &&                         // type
        !rlp_push_str(r, meth) &&                     // meth
        !rlp_push_str(r, path) &&                     // path
        (data ? !rlp_vpush_str(r, data, list) : true) // data
    ) {
        if (!(*buffer_p)) {
            *l = sz = rlp_print_size(r) + 1;
            *buffer_p = malloc(sz);
        }
        err = rlp_print(r, *buffer_p, l);
        add_terminator(*buffer_p, l, sz);
        rlp_free(&r);
        return err;
    } else {
        return -1;
    }
}

int
wire_parse_http_request(uint8_t* rlp, uint32_t l, wire_parser_http_request_s* r)
{
    int err = -1;
    uint32_t len, count;
    wire_parser_init(&r->wire);
    len = wire_parse(&r->wire, rlp, l);
    if (len == 0 && ((count = wire_count(&r->wire)) >= 4)) {
        r->vers = wire_parser_read_vers(&r->wire);
        r->type = wire_parser_read_type(&r->wire);
        r->meth = wire_parser_read_meth(&r->wire);
        r->path = wire_parser_read_path(&r->wire);
        r->data = (count > 4) ? wire_parser_read_data(&r->wire) : NULL;
        err = 0;
    } else {
        wire_parser_free(&r->wire);
        err = -1;
    }
    return err;
}

void
wire_parser_http_request_free(wire_parser_http_request_s* req)
{
    wire_parser_free(&req->wire);
}

int
wire_print_http_response(
    uint8_t* buffer,
    uint32_t* sz,
    uint16_t code,
    const char* message)
{
    return wire_print_http_response_ptr(&buffer, sz, code, message);
}

int
wire_print_http_response_alloc(
    uint8_t** buffer_p,
    uint32_t* l,
    uint16_t code,
    const char* message)
{
    assert(buffer_p);
    assert(*buffer_p == NULL);
    return wire_print_http_response_ptr(buffer_p, l, code, message);
}

int
wire_print_http_response_ptr(
    uint8_t** buffer_p,
    uint32_t* l,
    uint16_t code,
    const char* message)
{
    rlp* r;
    uint32_t sz = *l;
    int err;
    if ((r = rlp_list()) &&       //
        !rlp_push_u8(r, 0) &&     // vers
        !rlp_push_u8(r, 0) &&     // type
        !rlp_push_u16(r, code) && // code
        !rlp_push_str(r, message) // message
    ) {
        if (!(*buffer_p)) {
            *l = sz = rlp_print_size(r) + 1;
            *buffer_p = malloc(sz);
        }
        err = rlp_print(r, *buffer_p, l);
        add_terminator(*buffer_p, l, sz);
        rlp_free(&r);
        return err;
    } else {
        return -1;
    }
}

int
wire_parse_http_response(
    uint8_t* rlp,
    uint32_t l,
    wire_parser_http_response_s* r)
{
    int err = -1;
    uint32_t len, count;
    wire_parser_init(&r->wire);
    len = wire_parse(&r->wire, rlp, l);
    if (len == 0 && ((count = wire_count(&r->wire)) >= 4)) {
        r->vers = wire_parser_read_vers(&r->wire);
        r->type = wire_parser_read_type(&r->wire);
        r->code = wire_parser_read_code(&r->wire);
        r->mesg = wire_parser_read_mesg(&r->wire);
        err = 0;
    }
    return err;
}

void
wire_parser_http_response_free(wire_parser_http_response_s* res)
{
    wire_parser_free(&res->wire);
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

uint32_t
wire_count(wire_parser_s* wire)
{
    return rlp_children(wire->rlp);
}

void
wire_read_be(void* dat, int szofdat, int l, const uint8_t* b)
{
    static int test = 1;
    uint8_t* x = (&((uint8_t*)dat)[l - 1]);
    if (*(char*)&test == 0) {
        memcpy(dat, b, l);
    } else {
        while (l-- && szofdat--) {
            *x = *b++;
            x -= 1;
        }
        while (szofdat--) *x-- = 0;
    }
}

void
wire_write_be(uint8_t* b, const void* dat, int szofdat)
{
    static int test = 1;
    uint8_t* x;
    int inc, msb = 0, c = 0;
    if (*(char*)&test == 0) {
        x = (&((uint8_t*)dat)[0]);
        inc = 1;
    } else {
        x = (&((uint8_t*)dat)[szofdat - 1]);
        inc = -1;
    }
    while (szofdat--) {
        // We don't start populating dat until we hit msb as to ignore leading
        // zero's. So if msb or if *x has weight or if last byte we copy into
        // dat
        if (*x && !msb) msb = 1;
        if (msb || *x || !szofdat) { b[c++] = *x; }
        x += inc;
    }
}
uint32_t rlp_read_big_endian(void* dat, int szof, const uint8_t* b);

int
wire_read_sz(uint32_t* result, uint8_t* b, uint32_t l)
{
    // Returns 0 if success. Returns the amount of underflow if fail
    int ret = 0;
    uint8_t szofsz;
    assert(l);
    if (*b < 0x80) {
        *result = 1;
    } else if (*b <= 0xb7) {
        *result = 1 + *b - 0x80;
    } else if (*b <= 0xbf) {
        szofsz = *b - 0xb7;
        if (l <= szofsz + 1) {
            ret = szofsz + 1;
        } else {
            assert(szofsz <= 8);
            wire_read_be(result, sizeof(uint32_t), szofsz, ++b);
            *result += 1 + szofsz;
        }
    } else if (*b == 0xc0) {
        *result = 1;
    } else if (*b <= 0xf7) {
        *result = 1 + *b - 0xc0;
    } else {
        szofsz = *b - 0xf7;
        if (l <= szofsz + 1) {
            ret = szofsz + 1;
        } else {
            assert(szofsz <= 8);
            wire_read_be(result, sizeof(uint32_t), szofsz, ++b);
            *result += 1 + szofsz;
        }
    }
    return ret;
}
