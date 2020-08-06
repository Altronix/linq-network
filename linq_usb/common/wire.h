#ifndef WIRE_H
#define WIRE_H

#include "json.h"
#include "rlp.h"
#include "stdint.h"

#ifndef HAS_TERMINATE
#define HAS_TERMINATE TRUE
#endif

// clang-format off
#ifdef HAS_TERMINATE
#  ifndef WIRE_TERMINATE
#  define WIRE_TERMINATE '\n'
#  endif
#endif
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wire_parser_s
    {
        rlp* rlp;
    } wire_parser_s;

    typedef struct wire_parser_http_request_s
    {
        uint8_t vers;
        uint8_t type;
        const char* meth;
        const char* path;
        const char* data;
        wire_parser_s wire;
    } wire_parser_http_request_s;

    typedef struct wire_parser_http_response_s
    {
        uint8_t vers;
        uint8_t type;
        uint16_t code;
        const char* mesg;
        wire_parser_s wire;
    } wire_parser_http_response_s;

    int wire_print_http_request(
        uint8_t* buffer_p,
        uint32_t* l,
        const char* meth,
        const char* path,
        const char* data,
        ...);
    int wire_print_http_request_alloc(
        uint8_t** buffer_p,
        uint32_t* l,
        const char* meth,
        const char* path,
        const char* data,
        ...);
    int wire_print_http_request_ptr(
        uint8_t** buffer_p,
        uint32_t* l,
        const char* meth,
        const char* path,
        const char* data,
        va_list list);
    int wire_parse_http_request(
        uint8_t* rlp,
        uint32_t l,
        wire_parser_http_request_s* request);
    void wire_parser_http_request_free(wire_parser_http_request_s* req);
    int wire_print_http_response(
        uint8_t* buffer_p,
        uint32_t* l,
        uint16_t code,
        const char* message);
    int wire_print_http_response_alloc(
        uint8_t** buffer_p,
        uint32_t* l,
        uint16_t code,
        const char* message);
    int wire_print_http_response_ptr(
        uint8_t** buffer_p,
        uint32_t* l,
        uint16_t code,
        const char* message);
    int wire_parse_http_response(
        uint8_t* rlp,
        uint32_t l,
        wire_parser_http_response_s* response);
    void wire_parser_http_response_free(wire_parser_http_response_s* res);

    void wire_parser_init(wire_parser_s* wire);
    void wire_parser_free(wire_parser_s* wire);
    int wire_parse(wire_parser_s* wire, const uint8_t*, uint32_t l);
    uint32_t wire_count(wire_parser_s* wire);

#ifdef __cplusplus
}
#endif
#endif
