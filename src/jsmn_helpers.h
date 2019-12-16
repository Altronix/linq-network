#ifndef JSMN_HELPERS_H
#define JSMN_HELPERS_H
#ifdef __cplusplus
extern "C"
{
#endif

#define JSMN_HEADER
#include "jsmn/jsmn.h"
#include "sys.h"

    uint32_t jsmn_parse_tokens(
        const char* data,
        uint32_t n_tokens,
        jsmntok_t**,
        uint32_t,
        ...);
    uint32_t jsmn_parse_tokens_path(
        const char*,
        const char*,
        uint32_t,
        jsmntok_t**,
        uint32_t,
        ...);

#ifdef __cplusplus
}
#endif
#endif
