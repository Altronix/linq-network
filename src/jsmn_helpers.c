
#include "jsmn_helpers.h"
#include "altronix/linq_netw.h"
#include "sys.h"

static uint32_t
parse_token(const char* data, jsmntok_t* t, linq_str* result)
{
    result->len = t->end - t->start;
    result->p = &data[t->start];
    return result->len;
}

uint32_t
jsmn_parse_tokens(
    const char* data,
    uint32_t n_tokens,
    jsmntok_t** tokens_p,
    uint32_t n_tags,
    ...)
{
    char* cmp;
    linq_str tag;
    uint32_t count = 0;
    jsmntok_t* t = *tokens_p;
    for (uint32_t i = 0; i < n_tokens; i++) {
        if (t[i].type == JSMN_OBJECT || (t[i].type == JSMN_ARRAY)) {
            tag.p = NULL;
            continue;
        }
        if (!tag.p) {
            tag.len = parse_token(data, &t[i], &tag);
        } else {
            uint32_t c = n_tags << 1;
            va_list list;
            va_start(list, n_tags);
            while (c >= 2) {
                c -= 2;
                cmp = va_arg(list, char*);
                linq_str* result = va_arg(list, linq_str*);
                if (tag.len == strlen(cmp) && !memcmp(tag.p, cmp, tag.len)) {
                    parse_token(data, &t[i], result);
                    count++;
                }
            }
            va_end(list);
            tag.p = NULL;
        }
    }
    return count;
}

uint32_t
jsmn_parse_tokens_path(
    const char* data,
    const char* path,
    uint32_t n_tokens,
    jsmntok_t** tokens_p,
    uint32_t n_tags,
    ...)
{
    // Recursively navigate tokens till find last target
    // then call jsmn_parse_tokens()
    ((void)data);
    ((void)path);
    ((void)n_tokens);
    ((void)tokens_p);
    ((void)n_tags);
    // TODO
}
