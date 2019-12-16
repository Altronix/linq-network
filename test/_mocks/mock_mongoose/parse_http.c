// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "parse_http.h"
#include "mock_mongoose.h"

// Private
int http_response_on_header_field(http_parser* p, const char* at, size_t len);
int http_response_on_header_value(http_parser* p, const char* at, size_t len);
int http_response_on_url(http_parser* p, const char* at, size_t len);
int http_response_on_body(http_parser* p, const char* at, size_t len);
int http_response_on_status(http_parser* p, const char* at, size_t len);

int http_request_on_url(http_parser* p, const char* at, size_t len);
int http_request_on_body(http_parser* p, const char* at, size_t len);
int http_request_on_status(http_parser* p, const char* at, size_t len);

http_parser_settings parse_http_settings_response = {
    .on_header_field = http_response_on_header_field,
    .on_header_value = http_response_on_header_value,
    .on_url = http_response_on_url,
    .on_body = http_response_on_body,
    .on_status = http_response_on_status,
    .on_headers_complete = NULL,
    .on_message_begin = NULL,
    .on_message_complete = NULL,
    .on_chunk_header = NULL,
    .on_chunk_complete = NULL
};

http_parser_settings parse_http_settings_request = {
    .on_header_field = NULL,
    .on_header_value = NULL,
    .on_url = http_request_on_url,
    .on_body = http_request_on_body,
    .on_status = http_request_on_status,
    .on_headers_complete = NULL,
    .on_message_begin = NULL,
    .on_message_complete = NULL,
    .on_chunk_header = NULL,
    .on_chunk_complete = NULL
};

void
mongoose_parser_init(
    http_parser* parser,
    void* context,
    enum http_parser_type type)
{
    http_parser_init(parser, type);
    parser->data = context;
    memset(context, 0, sizeof(mongoose_parser_context));
}

int
mongoose_parser_parse(http_parser* parser, const char* data, uint32_t len)
{
    int ret;
    if (parser->type == HTTP_RESPONSE) {
        ret = http_parser_execute(
            parser, &parse_http_settings_response, data, len);
    } else {
        ret = http_parser_execute(
            parser, &parse_http_settings_request, data, len);
        const char* space = memchr(data, ' ', len);
        ((mock_mongoose_event*)parser->data)->message.message.p = data;
        ((mock_mongoose_event*)parser->data)->message.message.len = len;
        ((mock_mongoose_event*)parser->data)->message.method.p = data;
        ((mock_mongoose_event*)parser->data)->message.method.len = space - data;
    }
    return ret == (int)len ? 0 : -1;
}

int
http_response_on_header_field(http_parser* p, const char* at, size_t len)
{
    mongoose_parser_context* ctx = p->data;
    ctx->curr_header_ptr = at;
    ctx->curr_header_len = len;
    return 0;
}

int
http_response_on_header_value(http_parser* p, const char* at, size_t len)
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
http_response_on_url(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}

int
http_response_on_body(http_parser* p, const char* at, size_t len)
{
    int err = 0;
    mongoose_parser_context* ctx = p->data;
    if (len <= sizeof(ctx->body)) { memcpy(ctx->body, at, len); }
    return err;
}

int
http_response_on_status(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}

int
http_request_on_url(http_parser* p, const char* at, size_t len)
{
    mock_mongoose_event* req = p->data;
    const char* params = memchr(at, '?', len);
    req->message.uri.p = at;
    if (params) {
        req->message.uri.len = params - at;
        req->message.query_string.p = ++params;
        req->message.query_string.len = len - req->message.uri.len - 1;
    } else {
        req->message.uri.len = len;
    }
    return 0;
}

int
http_request_on_body(http_parser* p, const char* at, size_t len)
{
    mock_mongoose_event* req = p->data;
    req->message.body.p = at;
    req->message.body.len = len;
    return 0;
}

int
http_request_on_status(http_parser* p, const char* at, size_t len)
{
    ((void)p);
    ((void)at);
    ((void)len);
    return 0;
}

