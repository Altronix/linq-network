#include "altronix/linq_netw.h"
#include "sys.h"

#include "jsmn/jsmn.h"
#include "jsmn_helpers.h"

#define seek_slash(__bytes, __len, __result)                                   \
    do {                                                                       \
        while (__result < __len &&                                             \
               !(__bytes[__result] == '/' || __bytes[__result] == '\\'))       \
            __result++;                                                        \
                                                                               \
    } while (0)

static bool
map_key_to_value(
    const char* key,
    uint32_t keylen,
    const char* value,
    uint32_t value_len,
    uint32_t n_tags,
    va_list list)
{
    uint32_t c = n_tags << 1;
    char* cmp;
    while (c >= 2) {
        c -= 2;
        cmp = va_arg(list, char*);
        linq_str* result = va_arg(list, linq_str*);
        if (keylen == strlen(cmp) && !memcmp(key, cmp, keylen)) {
            result->len = value_len;
            result->p = value;
            return true;
        }
    }
    return false;
}

uint32_t
jsmn_parse_tokens(
    jsmntok_t* t,
    uint32_t max_tokens,
    const char* data,
    uint32_t sz,
    uint32_t n_tags,
    ...)
{
    uint32_t n_tokens;
    linq_str tag;
    jsmn_parser p;
    jsmn_init(&p);
    uint32_t count = 0;
    n_tokens = jsmn_parse(&p, data, sz, t, max_tokens);
    if (n_tokens <= max_tokens) {
        for (uint32_t i = 0; i < n_tokens; i++) {
            if (t[i].type == JSMN_OBJECT || (t[i].type == JSMN_ARRAY)) {
                tag.p = NULL;
                continue;
            }
            if (!tag.p) {
                tag.len = t[i].end - t[i].start;
                tag.p = &data[t[i].start];
            } else {
                va_list list;
                va_start(list, n_tags);
                count += map_key_to_value(
                             tag.p,
                             tag.len,
                             &data[t[i].start],
                             t[i].end - t[i].start,
                             n_tags,
                             list)
                             ? 1
                             : 0;
                va_end(list);
                tag.p = NULL;
            }
        }
    }
    return count;
}

uint32_t
jsmn_parse_tokens_path(
    const char* path,
    jsmntok_t* t,
    uint32_t max_tokens,
    const char* data,
    uint32_t sz,
    uint32_t n_tags,
    ...)
{
    uint32_t n_tokens, count = 0;
    jsmn_parser p;
    jsmn_init(&p);
    n_tokens = jsmn_parse(&p, data, sz, t, max_tokens);
    if (!(n_tokens <= max_tokens)) return count;

    jsmntok_t* curr = t;
    uint32_t parent = 0, i = 0, spot = 0, cmplen;

    while (i < max_tokens) {
        if (t[i].type == JSMN_OBJECT) {
            parent = i;
            i++;
        }
        if (t[i].type == JSMN_STRING) {
            if (*path == '/' || *path == '\\') path++;
            seek_slash(path, strlen(path), spot);
            cmplen = t[i].end - t[i].start;
            if (spot == cmplen && !memcmp(path, &data[t[i].start], spot)) {
                path += spot;
                i++;
                if (!path) {
                    // Found the object we are looking for...
                }
            } else {
                if (!(t[parent].end == (int)sz)) {
                    while (t[i].start < t[parent].end) { i++; }
                } else {
                    i++;
                }
            }
        } else {
            // JSMN_UNDEFINED || JSMN_ARRAY || JSMN_PRIMITIVE ?
            i++;
        }
    }
}
