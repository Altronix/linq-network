#ifndef JSMN_HELPERS_H
#define JSMN_HELPERS_H

#ifdef linux
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __jsmn_malloc_fn malloc
#define __jsmn_free_fn free
#define __jsmn_assert_fn assert
#define __jsmn_malloc(x) __jsmn_malloc_fn(x)
#define __jsmn_free(x) __jsmn_free_fn(x)
#define __jsmn_assert(x) __jsmn_assert_fn(x)

#elif defined(_WIN32) || defined(WIN32)
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define __jsmn_malloc_fn malloc
#define __jsmn_free_fn free
#define __jsmn_assert_fn assert
#define __jsmn_malloc(x) __jsmn_malloc_fn(x)
#define __jsmn_free(x) __jsmn_free_fn(x)
#define __jsmn_assert(x) __jsmn_assert_fn(x)

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define JSMN_HEADER
#include "jsmn.h"

#define jsmn_helper(x) jsmntok_t[x]

    typedef struct jsmn_helpers_value
    {
        const char* p;
        uint32_t len;
    } jsmn_helpers_value;

    typedef unsigned char byte;

    void jsmn_foreach(
        jsmntok_t* t,
        uint32_t n_tokens,
        const char* data,
        void (*cb)(void*, jsmn_helpers_value* key, jsmn_helpers_value* val),
        void*);

    uint32_t jsmn_parse_tokens(
        jsmntok_t*,
        uint32_t sz,
        const char* data,
        uint32_t,
        uint32_t,
        ...);

    uint32_t jsmn_parse_tokens_path(
        const char* path,
        jsmntok_t*,
        uint32_t sz,
        const char* data,
        uint32_t,
        uint32_t,
        ...);

#ifdef __cplusplus
}
#endif
#endif
