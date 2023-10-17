#ifndef JSON_H
#define JSON_H

#include "sys.h"

#define JSMN_HEADER
#include "jsmn.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef jsmn_parser json_parser;
    typedef jsmntok_t jsontok;
    typedef struct json_value
    {
        const char* p;
        uint32_t len;
    } json_value;

    void json_init(jsmn_parser* p);
    int json_parse(
        jsmn_parser* parser,
        const char* js,
        const size_t len,
        jsmntok_t* tokens,
        const unsigned int num_tokens);
    void json_parse_int_tok(const char* buff, const jsontok* t, int* result);
    int json_parse_int(
        const char* buff,
        jsontok* toks,
        const char* guide,
        int* result);
    int json_parse_value(
        const char* buff,
        jsontok* toks,
        const char* guide,
        json_value* v);
    void
    json_parse_value_tok(const char* buff, const jsontok* t, json_value* v);
    bool json_tok_is_null(const char* buffer, const jsmntok_t* tok);
    const jsmntok_t* json_next(const jsmntok_t* tok);
    bool
    json_tok_streq(const char* buffer, const jsmntok_t* tok, const char* str);
    const jsmntok_t* json_get_member(
        const char* buffer,
        const jsmntok_t tok[],
        const char* label);
    const jsmntok_t* json_get_arr(const jsmntok_t tok[], size_t index);
    json_value json_tok_value(const char* buf, const jsmntok_t* tok);
    const jsmntok_t*
    json_delve(const char* buf, const jsmntok_t* tok, const char* guide);
    json_value
    json_delve_value(const char* buff, jsmntok_t* tok, const char* guide);
    int json_to_u64(const char* buf, const jsmntok_t* tok, uint64_t* n);
    int json_to_s64(const char* buf, const jsmntok_t* tok, int64_t* n);
    int json_to_u32(const char* buf, const jsmntok_t* tok, uint32_t* n);
    int json_to_s32(const char* buf, const jsmntok_t* tok, int32_t* n);
    int json_to_u16(const char* buf, const jsmntok_t* tok, uint16_t* n);
    int json_to_s16(const char* buf, const jsmntok_t* tok, int16_t* n);
    int json_to_u8(const char* buf, const jsmntok_t* tok, uint8_t* n);
    int json_to_s8(const char* buf, const jsmntok_t* tok, int8_t* n);

#ifdef __cplusplus
}
#endif
#endif /* JSON_H */
