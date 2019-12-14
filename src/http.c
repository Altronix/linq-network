#include "http.h"
#include "log.h"

#define HTTP_FORMAT_HEADERS                                                    \
    "HTTP/1.0 %d \r\n"                                                         \
    "Server: Linq Embedded Web Server\r\n"                                     \
    "Connection: %s\r\n"                                                       \
    "Content-Type: %s\r\n"                                                     \
    "Content-Length: %d\r\n"                                                   \
    "%s"                                                                       \
    "X-Frame-Options: SAMEORIGIN\r\n"                                          \
    "Content-Security-Policy: script-src 'self' 'unsafe-eval';\r\n"            \
    "\r\n"

// Hash map of URL's to routes
MAP_INIT_W_FREE(routes, http_route_context);

static inline HTTP_METHOD
get_method(struct http_message* m)
{
    if (m->method.p[0] == 'P') {
        return m->method.p[1] == 'U' ? HTTP_PUT : HTTP_POST;
    } else if (m->method.p[0] == 'D') {
        return HTTP_DELETE;
    } else {
        return HTTP_GET;
    }
}

static inline const char*
get_uri(struct http_message* m)
{
    static char path[128];
    if (!(m->uri.len < sizeof(path))) {
        return NULL;
    } else {
        path[m->uri.len] = '\0';
        memcpy(path, m->uri.p, m->uri.len);
        return path;
    }
}

static void
http_ev_handler(struct mg_connection* c, int ev, void* p)
{
    switch (ev) {
        case MG_EV_POLL: break;
        case MG_EV_ACCEPT: log_info("%06s %04s", "(HTTP)", "Accept"); break;
        case MG_EV_CONNECT: log_info("%06s %04s", "(HTTP)", "Connect"); break;
        case MG_EV_RECV: log_info("%06s %04s", "(HTTP)", "Recv"); break;
        case MG_EV_SEND: log_info("%06s %04s", "(HTTP)", "Send"); break;
        case MG_EV_CLOSE: log_info("%06s %04s", "(HTTP)", "Close"); break;
        case MG_EV_TIMER: log_info("%06s %04s", "(HTTP)", "Timer"); break;
        case MG_EV_HTTP_REQUEST: {
            http_route_context** r;
            http_s* http = c->user_data;
            struct http_message* m = (struct http_message*)p;
            const char* path = get_uri(m);
            if (path && (r = routes_map_get(http->routes, path))) {
                log_info("%06s %04s", "(HTTP) %s (%s)", "Req.", path);
                (*r)->cb((*r)->context, get_method(m), m->body.len, m->body.p);
            } else {
                log_info("%06s %04s", "(HTTP) %s (404)", "Req.");
                http_printf_json(c, 404, "{\"error\":\"%s\"}", "not found");
            }
        } break;
        case MG_EV_HTTP_REPLY: log_info("%06s %04s", "(HTTP)", "Reply"); break;
        case MG_EV_HTTP_CHUNK: log_info("%06s %04s", "(HTTP)", "Chunk"); break;
        case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
        case MG_EV_WEBSOCKET_FRAME:
        case MG_EV_WEBSOCKET_CONTROL_FRAME:
            log_info("%06s %04s", "(HTTP)", "Websocket ctrl frame");
            break;
        case MG_EV_HTTP_MULTIPART_REQUEST:
        case MG_EV_HTTP_PART_BEGIN:
        case MG_EV_HTTP_PART_DATA:
        case MG_EV_HTTP_PART_END:
        case MG_EV_HTTP_MULTIPART_REQUEST_END:
            log_info("%06s %04s", "(HTTP)", "Recv");
            break;
        default:
            log_error("%06s %04s %s (%d)", "(HTTP)", "Recv", "Unkown", ev);
            break;
    }
}

void
http_init(http_s* http)
{
    memset(http, 0, sizeof(http_s));
    mg_mgr_init(&http->connections, http);
    http->routes = routes_map_create();
}

void
http_deinit(http_s* http)
{
    mg_mgr_free(&http->connections);
    routes_map_destroy(&http->routes);
}

E_LINQ_ERROR
http_poll(http_s* http, int32_t ms)
{
    int err = mg_mgr_poll(&http->connections, ms);
    return err;
}

void
http_listen(http_s* http, const char* port)
{
    if (http->listener) {
        log_fatal("%10s", "HTTP can only listen to one server at a time!");
        log_fatal("%10s", "Please shutdown HTTP server before listening again");
        linq_netw_assert(http->listener);
    }
    http->listener = mg_bind(&http->connections, port, http_ev_handler);
    mg_set_protocol_http_websocket(http->listener);
}

void
http_use(http_s* http, const char* path, http_route_cb cb, void* context)
{
    http_route_context* ctx = linq_netw_malloc(sizeof(http_route_context));
    linq_netw_assert(ctx);
    ctx->cb = cb;
    ctx->context = context;
    routes_map_add(http->routes, path, &ctx);
}

void
http_printf_json(void* c, int code, const char* fmt, ...)
{
    size_t l;
    va_list ap;

    // Get Content-Length
    va_start(ap, fmt);
    l = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    // Send data
    va_start(ap, fmt);
    http_vprintf(c, code, "application/json", l, fmt, ap);
    va_end(ap);
}

void
http_printf(void* connection, int code, const char* type, const char* fmt, ...)
{
    size_t l;
    va_list ap;

    // Get Content-Length
    va_start(ap, fmt);
    l = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    // Send data
    va_start(ap, fmt);
    http_vprintf(connection, code, type, l, fmt, ap);
    va_end(ap);
}

void
http_vprintf(
    void* c,
    int code,
    const char* type,
    uint32_t l,
    const char* fmt,
    va_list list)
{
    // Send headers
    mg_printf(c, HTTP_FORMAT_HEADERS, code, "keep-alive", type, l, "");
    mg_vprintf(c, fmt, list);
}
