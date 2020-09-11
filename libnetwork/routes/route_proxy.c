#include "netw.h"
#include "routes.h"

#define API_URI "/api/v1/proxy/"
#define API_URI_LEN (sizeof(API_URI) - 1)

static void
cb(void* ctx, const char* serial, E_LINQ_ERROR error, const char* json)
{
    http_request_s* r = ctx;
    http_resolve_json(r, http_error_code(error), "%s", json);
}

void
route_proxy(
    http_request_s* r,
    HTTP_METHOD meth,
    uint32_t jlen,
    const char* body)
{
    char serial[64];
    uint32_t plen;
    struct http_message* m = (*r->route_p)->curr_message;
    const char *url = &m->uri.p[API_URI_LEN], *ptr = url;
    netw_s* linq = http_request_context(r);
    ptr = memchr(url, '/', m->uri.len - API_URI_LEN);
    if (!ptr) ptr = memchr(url, '\\', m->uri.len - API_URI_LEN);
    if (ptr) {
        snprintf(serial, sizeof(serial), "%.*s", (int)(ptr - url), url);
        plen = m->uri.len - (ptr - m->uri.p);
        if (meth == HTTP_METHOD_POST || meth == HTTP_METHOD_PUT) {
            netw_send(linq, serial, "POST", ptr, plen, body, jlen, cb, r);
            r->more = true;
        } else if (meth == HTTP_METHOD_DELETE) {
            netw_send(linq, serial, "DELETE", ptr, plen, NULL, 0, cb, r);
            r->more = true;
        } else {
            netw_send(linq, serial, "GET", ptr, plen, NULL, 0, cb, r);
            r->more = true;
        }
    } else {
        http_printf_json(r->connection, 400, JERROR_400);
    }
}
