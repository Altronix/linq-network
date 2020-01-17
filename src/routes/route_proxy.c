#include "routes.h"

#include "device.h"

#define API_URI "/api/v1/linq-lite/proxy/"
#define API_URI_LEN (sizeof(API_URI) - 1)

static void
on_response(void* ctx, E_LINQ_ERROR error, const char* json, device_s** device)
{
    ((void)device);
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
    atx_net_s* linq = ctx->context;
    ptr = memchr(url, '/', ctx->curr_message->uri.len - API_URI_LEN);
    if (!ptr) ptr = memchr(url, '\\', ctx->curr_message->uri.len - API_URI_LEN);
    if (ptr) {
        snprintf(serial, sizeof(serial), "%.*s", (int)(ptr - url), url);
        device_s** d_p = atx_net_device(linq, serial);
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
            http_printf_json(
                ctx->curr_connection, 404, "{\"error\":\"Device not found\"}");
        }
    } else {
        http_printf_json(
            ctx->curr_connection, 400, "{\"error\":\"Bad request\"}");
    }
    // ctx->curr_message->
    // http_printf_json(ctx, 504, "{\"TODO\":\"TODO\"}");
}

