#include "altronix/linq_netw.h"
#include "sys.h"

#include "jsmn/jsmn.h"
#include "jsmn_helpers.h"

static uint32_t
parse_token(const char* data, jsmntok_t* t, linq_str* result)
{
    result->len = t->end - t->start;
    result->p = &data[t->start];
    return result->len;
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
    char* cmp;
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
                tag.len = parse_token(data, &t[i], &tag);
            } else {
                uint32_t c = n_tags << 1;
                va_list list;
                va_start(list, n_tags);
                while (c >= 2) {
                    c -= 2;
                    cmp = va_arg(list, char*);
                    linq_str* result = va_arg(list, linq_str*);
                    if (tag.len == strlen(cmp) &&
                        !memcmp(tag.p, cmp, tag.len)) {
                        parse_token(data, &t[i], result);
                        count++;
                    }
                }
                va_end(list);
                tag.p = NULL;
            }
        }
    }
    return count;
}

uint32_t
jsmn_parse_tokens_path(
    jsmntok_t* t,
    uint32_t max_tokens,
    const char* data,
    uint32_t sz,
    uint32_t n_tags,
    ...)
{
    ((void)t);
    ((void)max_tokens);
    ((void)data);
    ((void)sz);
    ((void)n_tags);
}
