#ifndef WIRE_H
#define WIRE_H

#include "json.h"
#include "rlp.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wire_parser_s
    {
        rlp* rlp;
    } wire_parser_s;

    int wire_print_buffer(
        uint8_t* buffer_p,
        uint32_t* l,
        const char* meth,
        const char* path,
        const char* data);
    int wire_print(
        uint8_t** buffer_p,
        uint32_t* l,
        const char* meth,
        const char* path,
        const char* data);

    void wire_parser_init(wire_parser_s* wire);
    void wire_parser_free(wire_parser_s* wire);
    int wire_parse(wire_parser_s* wire, const uint8_t*, uint32_t l);
    uint32_t wire_count(wire_parser_s* wire);
    uint8_t wire_parser_read_vers(wire_parser_s* wire);
    uint8_t wire_parser_read_type(wire_parser_s* wire);
    const char* wire_parser_read_meth(wire_parser_s* wire);
    const char* wire_parser_read_path(wire_parser_s* wire);
    const char* wire_parser_read_data(wire_parser_s* wire);

#ifdef __cplusplus
}
#endif
#endif
