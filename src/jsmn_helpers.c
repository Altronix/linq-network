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
is_value(jsmntok_t* t, const char* data, const char** str_p, uint32_t* len)
{
    if (t->type == JSMN_OBJECT || t->type == JSMN_ARRAY) {
        // This is an object so we don't know the name of the tag yet
        *str_p = NULL, *len = 0;
        return false;
    } else if (!*str_p) {
        // This is the key name, but not the value. Populate key
        *str_p = &data[t->start];
        *len = t->end - t->start;
        return false;
    } else {
        // This token is pointing at the value and previous token is point to
        // the key
        return true;
    }
}

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
    if (!(n_tokens <= max_tokens)) return 0;
    for (uint32_t i = 0; i < n_tokens; i++) {
        if (!is_value(&t[i], data, &tag.p, &tag.len)) continue;
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

    uint32_t parent = 0, i = 0, spot = 0, cmplen;

    while (i < n_tokens) {
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
                parent = i;
                if (path && !*path) {
                    // Found the object we are looking for...
                    const char* tag;
                    uint32_t taglen;
                    while (i < n_tokens && t[i].end <= t[parent].end) {
                        if (!is_value(&t[i], data, &tag, &taglen)) {
                            i++;
                            continue;
                        }
                        va_list list;
                        va_start(list, n_tags);
                        count += map_key_to_value(
                                     tag,
                                     taglen,
                                     &data[t[i].start],
                                     t[i].end - t[i].start,
                                     n_tags,
                                     list)
                                     ? 1
                                     : 0;
                        va_end(list);
                        i++;
                        tag = NULL;
                    }
                    break;
                }
            } else {
                if (!(++i < n_tokens)) break;
                int end = t[i].end;
                while (t[i].start < end) {
                    i++;
                    //
                }
                // if (!(t[parent].end == (int)sz)) {
                //     while (t[i].start < t[parent].end) { i++; }
                // } else {
                //     i++;
                // }
            }
        } else {
            // JSMN_UNDEFINED || JSMN_ARRAY || JSMN_PRIMITIVE ?
            i++;
        }
    }
    return count;
}
