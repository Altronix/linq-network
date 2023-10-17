#include "jsmn.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

#define json_for_each_obj(i, t, obj)                                           \
    for (i = 0, t = (obj) + 1; i < (obj)->size; t = json_next(t + 1), i++)
#define json_for_each_arr(i, t, arr)                                           \
    for (i = 0, t = (arr) + 1; i < (arr)->size; t = json_next(t), i++)

void
json_init(jsmn_parser* p)
{
    jsmn_init(p);
}

int
json_parse(
    jsmn_parser* parser,
    const char* js,
    const size_t len,
    jsmntok_t* tokens,
    const unsigned int num_tokens)
{
    return jsmn_parse(parser, js, len, tokens, num_tokens);
}

int
json_parse_int(const char* buff, jsontok* toks, const char* guide, int* result)
{
    const jsontok* t = json_delve(buff, toks, guide);
    if (t) {
        json_parse_int_tok(buff, t, result);
        return 0;
    } else {
        return -1;
    }
}
void
json_parse_int_tok(const char* buff, const jsontok* t, int* result)
{
    char b[12];
    snprintf(b, sizeof(b), "%.*s", t->end - t->start, &buff[t->start]);
    *result = atoi(b);
}

int
json_parse_value(
    const char* buff,
    jsontok* toks,
    const char* guide,
    json_value* v)
{
    const jsontok* t = json_delve(buff, toks, guide);
    if (t) {
        json_parse_value_tok(buff, t, v);
        return 0;
    } else {
        return -1;
    }
}

void
json_parse_value_tok(const char* buff, const jsontok* t, json_value* v)
{
    *v = json_tok_value(buff, t);
}

bool
json_tok_is_null(const char* buffer, const jsmntok_t* tok)
{
    if (tok->type != JSMN_PRIMITIVE) return false;
    return buffer[tok->start] == 'n';
}

const jsmntok_t*
json_next(const jsmntok_t* tok)
{
    const jsmntok_t* t;
    size_t i;

    for (t = tok + 1, i = 0; i < tok->size; i++) t = json_next(t);

    return t;
}

bool
json_tok_streq(const char* buffer, const jsmntok_t* tok, const char* str)
{
    if (tok->type != JSMN_STRING) return false;
    if (tok->end - tok->start != strlen(str)) return false;
    return strncmp(buffer + tok->start, str, tok->end - tok->start) == 0;
}

const jsmntok_t*
json_get_member(const char* buffer, const jsmntok_t tok[], const char* label)
{
    const jsmntok_t* t;
    size_t i;

    if (tok->type != JSMN_OBJECT) return NULL;

    json_for_each_obj(i, t, tok)
    {
        if (json_tok_streq(buffer, t, label)) return t + 1;
    }

    return NULL;
}

const jsmntok_t*
json_get_arr(const jsmntok_t tok[], size_t index)
{
    const jsmntok_t* t;
    size_t i;

    if (tok->type != JSMN_ARRAY) return NULL;

    json_for_each_arr(i, t, tok)
    {
        if (index == 0) return t;
        index--;
    }

    return NULL;
}

json_value
json_tok_value(const char* b, const jsmntok_t* t)
{
    json_value ret;
    ret.p = &b[t->start];
    ret.len = t->end - t->start;
    return ret;
}

const jsmntok_t*
json_delve(const char* buf, const jsmntok_t* tok, const char* guide)
{
    char key[128];
    while (*guide) {
        int len, sz = strcspn(guide + 1, ".");
        len = snprintf(key, sizeof(key), "%.*s", sz, guide + 1);
        assert(len < sizeof(key));
        switch (guide[0]) {
            case '.':
                if (!(tok->type == JSMN_OBJECT)) return NULL;
                tok = json_get_member(buf, tok, key);
                if (!tok) return NULL;
                break;
            default: abort();
        }
        guide += sz + 1;
    }
    return tok;
}

json_value
json_delve_value(const char* buff, jsmntok_t* tok, const char* guide)
{
    json_value ret = { .p = NULL, .len = 0 };
    const jsmntok_t* t = json_delve(buff, tok, guide);
    return t ? json_tok_value(buff, t) : ret;
}

int
json_to_u64(const char* buf, const jsmntok_t* tok, uint64_t* n)
{
    char* end;
    unsigned long long l;
    l = strtoull(buf + tok->start, &end, 0);
    if (end != buf + tok->end) return false;
    assert(sizeof(l) >= sizeof(*n));
    *n = l;
    if ((l == ULLONG_MAX) && errno == ERANGE) return -1;
    return *n == l ? 0 : -1;
}

int
json_to_s64(const char* buf, const jsmntok_t* tok, int64_t* n)
{
    char* end;
    unsigned long long l;
    l = strtoull(buf + tok->start, &end, 0);
    if (end != buf + tok->end) return false;
    assert(sizeof(l) >= sizeof(*n));
    *n = l;
    if ((l == LONG_MAX || l == LONG_MIN) && errno == ERANGE) return -1;
    return *n == l ? 0 : -1;
}

#define make_json_to_u(id, type)                                               \
    int json_to_##id(const char* buffer, const jsmntok_t* tok, type* ret)      \
    {                                                                          \
        uint64_t x;                                                            \
        if (json_to_u64(buffer, tok, &x)) return -1;                           \
        *ret = x;                                                              \
        return *ret == x ? 0 : -1;                                             \
    }

#define make_json_to_s(id, type)                                               \
    int json_to_##id(const char* buffer, const jsmntok_t* tok, type* ret)      \
    {                                                                          \
        int64_t x;                                                             \
        if (json_to_s64(buffer, tok, &x)) return -1;                           \
        *ret = x;                                                              \
        return *ret == x ? 0 : -1;                                             \
    }
make_json_to_u(u32, uint32_t);
make_json_to_s(s32, int32_t);
make_json_to_u(u16, uint16_t);
make_json_to_s(s16, int16_t);
make_json_to_u(u8, uint8_t);
make_json_to_s(s8, int8_t);
