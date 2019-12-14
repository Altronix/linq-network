#include "parse_http_response.h"

// Private
int parse_http_on_header_field(http_parser* p, const char* at, size_t len);
int parse_http_on_header_value(http_parser* p, const char* at, size_t len);
int parse_http_on_url(http_parser* p, const char* at, size_t len);
int parse_http_on_body(http_parser* p, const char* at, size_t len);
int parse_http_on_status(http_parser* p, const char* at, size_t len);

http_parser_settings parse_http_settings = { .on_header_field =
                                                 parse_http_on_header_field,
                                             .on_header_value =
                                                 parse_http_on_header_value,
                                             .on_url = parse_http_on_url,
                                             .on_body = parse_http_on_body,
                                             .on_status = parse_http_on_status,
                                             .on_headers_complete = NULL,
                                             .on_message_begin = NULL,
                                             .on_message_complete = NULL,
                                             .on_chunk_header = NULL,
                                             .on_chunk_complete = NULL };

void
mongoose_parser_init(http_parser* parser, mongoose_parser_context* context)
{
    http_parser_init(parser, HTTP_RESPONSE);
    parser->data = context;
    memset(context, 0, sizeof(mongoose_parser_context));
}

int
mongoose_parser_parse(http_parser* parser, const char* data, uint32_t len)
{
    int ret;
    ret = http_parser_execute(parser, &parse_http_settings, data, len);
    return ret == (int)len ? 0 : -1;
}

int
parse_http_on_header_field(http_parser* p, const char* at, size_t len)
{
    mongoose_parser_context* ctx = p->data;
    ctx->curr_header_ptr = at;
    ctx->curr_header_len = len;
    return 0;
}

int
parse_http_on_header_value(http_parser* p, const char* at, size_t len)
{
    int err = 0;
    mongoose_parser_context* ctx = p->data;
    if (len < 32) {
        if ((ctx->curr_header_len == 14) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Length", 14)))) {
            ctx->content_length = atoi(at);
        } else if (
            (ctx->curr_header_len == 12) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Type", 12)))) {
            memcpy(ctx->content_type, at, len);
            ctx->content_type[len] = 0;
        } else if (
            (ctx->curr_header_len == 16) &&
            (!(memcmp(ctx->curr_header_ptr, "Content-Encoding", 16)))) {
            memcpy(ctx->content_encoding, at, len);
            ctx->content_encoding[len] = 0;
        } else if (
            (ctx->curr_header_len == 6) &&
            (!(memcmp(ctx->curr_header_ptr, "Server", 6)))) {
            memcpy(ctx->server, at, len);
            ctx->server[len] = 0;
        } else if (
            (ctx->curr_header_len == 10) &&
            (!(memcmp(ctx->curr_header_ptr, "Connection", 10)))) {
            memcpy(ctx->connection, at, len);
            ctx->connection[len] = 0;
        } else if (
            (ctx->curr_header_len == 13) &&
            (!(memcmp(ctx->curr_header_ptr, "Cache-Control", 13)))) {
            memcpy(ctx->cache_control, at, len);
            ctx->cache_control[len] = 0;
        }
    }
    return err;
}

int
parse_http_on_url(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}

int
parse_http_on_body(http_parser* p, const char* at, size_t len)
{
    int err = 0;
    mongoose_parser_context* ctx = p->data;
    if (len <= sizeof(ctx->body)) { memcpy(ctx->body, at, len); }
    return err;
}

int
parse_http_on_status(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}
