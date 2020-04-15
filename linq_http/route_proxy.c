#include "routes.h"

#include "device.h"

#define API_URI "/api/v1/proxy/"
#define API_URI_LEN (sizeof(API_URI) - 1)

static void
on_response(void* ctx, const char* serial, E_LINQ_ERROR error, const char* json)
{
    struct mg_connection* connection = ctx;
    http_printf_json(connection, error, json);
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
    linq_network_s* linq = ((http_s*)ctx->context)->linq;
    ptr = memchr(url, '/', ctx->curr_message->uri.len - API_URI_LEN);
    if (!ptr) ptr = memchr(url, '\\', ctx->curr_message->uri.len - API_URI_LEN);
    if (ptr) {
        snprintf(serial, sizeof(serial), "%.*s", (int)(ptr - url), url);
        device_s** d_p = linq_network_device(linq, serial);
        if (d_p) {
            plen =
                ctx->curr_message->uri.len - (ptr - ctx->curr_message->uri.p);
            if (meth == HTTP_METHOD_POST || meth == HTTP_METHOD_PUT) {
                device_send_post_mem(
                    *d_p,
                    ptr,
                    plen,
                    body,
                    jlen,
                    on_response,
                    ctx->curr_connection);
            } else if (meth == HTTP_METHOD_DELETE) {
                device_send_delete_mem(
                    *d_p, ptr, plen, on_response, ctx->curr_connection);
            } else {
                device_send_get_mem(
                    *d_p, ptr, plen, on_response, ctx->curr_connection);
            }
        } else {
            http_printf_json(ctx->curr_connection, 404, JERROR_404);
        }
    } else {
        http_printf_json(ctx->curr_connection, 400, JERROR_400);
    }
    // ctx->curr_message->
    // http_printf_json(ctx, 504, "{\"TODO\":\"TODO\"}");
}

