
#include "jsmn_helpers.h"
#include "sys.h"

static uint32_t
parse_token(char* data, jsmntok_t* t, jsmn_result* result)
{
    result->len = t->end - t->start;
    result->p = &data[t->start];
    return result->len;
}

static uint32_t
jsmn_parse_tokens(
    char* data,
    uint32_t n_tokens,
    jsmntok_t** tokens_p,
    uint32_t n_tags,
    ...)
{
    char* cmp;
    jsmn_result tag;
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
                jsmn_result* result = va_arg(list, jsmn_result*);
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
