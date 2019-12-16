#ifndef JSMN_HELPERS_H
#define JSMN_HELPERS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "jsmn/jsmn.h"
#include "sys.h"

    typedef struct jsmn_result
    {
        const char* p;
        uint32_t len;
    } jsmn_result;

    static uint32_t parse_tokens(char*, uint32_t, jsmntok_t**, uint32_t, ...);

#ifdef __cplusplus
}
#endif
#endif
