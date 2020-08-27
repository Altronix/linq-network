#include "netw.h"
#include "routes.h"

#define API_URI "/api/v1/proxy/"
#define API_URI_LEN (sizeof(API_URI) - 1)

static void
on_response(void* ctx, const char* serial, E_LINQ_ERROR error, const char* json)
{
    struct mg_connection* connection = ctx;
    http_printf_json(connection, http_error_code(error), "%s", json);
}

void
route_proxy(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t jlen,
    const char* body)
{
    char serial[64];
    uint32_t plen;
    const char *url = &ctx->curr_message->uri.p[API_URI_LEN], *ptr = url;
    netw_s* linq = ctx->context;
    ptr = memchr(url, '/', ctx->curr_message->uri.len - API_URI_LEN);
    if (!ptr) ptr = memchr(url, '\\', ctx->curr_message->uri.len - API_URI_LEN);
    if (ptr) {
        snprintf(serial, sizeof(serial), "%.*s", (int)(ptr - url), url);
        plen = ctx->curr_message->uri.len - (ptr - ctx->curr_message->uri.p);
        if (meth == HTTP_METHOD_POST || meth == HTTP_METHOD_PUT) {
            netw_send(
                linq,
                serial,
                "POST",
                ptr,
                plen,
                body,
                jlen,
                on_response,
                ctx->curr_connection);
        } else if (meth == HTTP_METHOD_DELETE) {
            netw_send(
                linq,
                serial,
                "DELETE",
                ptr,
                plen,
                NULL,
                0,
                on_response,
                ctx->curr_connection);
        } else {
            netw_send(
                linq,
                serial,
                "GET",
                ptr,
                plen,
                NULL,
                0,
                on_response,
                ctx->curr_connection);
        }
    } else {
        http_printf_json(ctx->curr_connection, 400, JERROR_400);
    }
}

