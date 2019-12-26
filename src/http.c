// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

// If caller sends us variadic format string with out the length,
// we use this to get the length prior to sending
#define va_get_len(__list, __fmt, result)                                      \
    va_start(__list, __fmt);                                                   \
    result = vsnprintf(NULL, 0, __fmt, __list);                                \
    va_end(__list);

// Hash map of URL's to routes
MAP_INIT_W_FREE(routes, http_route_context);

static inline HTTP_METHOD
get_method(struct http_message* m)
{
    if (m->method.p[0] == 'P') {
        return m->method.p[1] == 'U' ? HTTP_METHOD_PUT : HTTP_METHOD_POST;
    } else if (m->method.p[0] == 'D') {
        return HTTP_METHOD_DELETE;
    } else {
        return HTTP_METHOD_GET;
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

// Write to connection
static void
c_vprintf(
    struct mg_connection* c,
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

// Write json to connection
static void
c_printf_json(struct mg_connection* c, int code, const char* fmt, ...)
{
    size_t l;
    va_list ap;

    // Send data
    va_get_len(ap, fmt, l);
    va_start(ap, fmt);
    c_vprintf(c, code, "application/json", l, fmt, ap);
    va_end(ap);
}

static void
c_printf(void* connection, int code, const char* type, const char* fmt, ...)
{
    size_t l;
    va_list ap;

    // Send data
    va_get_len(ap, fmt, l);
    va_start(ap, fmt);
    c_vprintf(connection, code, type, l, fmt, ap);
    va_end(ap);
}

typedef struct foreach_route_check_path_context
{
    const char* path;
    http_route_context*** found_p;
} foreach_route_check_path_context;

static void
foreach_route_check_path(
    routes_map_s* self,
    void* data,
    const char* test_path,
    http_route_context** r_p)
{
    ((void)self);
    foreach_route_check_path_context* ctx = data;
    if (!*ctx->found_p) {
        uint32_t plen = strlen(ctx->path), tlen = strlen(test_path);
        if (tlen >= 4 && !(memcmp(&test_path[tlen - 4], "/...", 4))) {
            if (plen > tlen && !(memcmp(test_path, ctx->path, tlen - 4))) {
                *ctx->found_p = r_p;
            }
        }
    }
}

static http_route_context**
resolve_route(http_s* http, const char* path)
{
    http_route_context** r_p = NULL;
    foreach_route_check_path_context ctx = { .path = path, .found_p = &r_p };
    if (path) r_p = routes_map_get(http->routes, path);
    if (!r_p) routes_map_foreach(http->routes, foreach_route_check_path, &ctx);
    return r_p;
}

static void
http_ev_handler(struct mg_connection* c, int ev, void* p, void* user_data)
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
            http_s* http = user_data;
            struct http_message* m = (struct http_message*)p;
            const char* path = get_uri(m);
            http_route_context** r = resolve_route(http, path);
            if (r) {
                (*r)->curr_connection = c;
                (*r)->curr_message = m;
                (*r)->cb(*r, get_method(m), m->body.len, m->body.p);
            } else {
                log_info("%06s %04s %s [%s]", "(HTTP)", "Req.", "(404)", path);
                c_printf_json(c, 404, "{\"error\":\"%s\"}", "not found");
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
    http->listener = mg_bind(&http->connections, port, http_ev_handler, http);
    mg_set_protocol_http_websocket(http->listener);
}

void
http_use(http_s* http, const char* path, http_route_cb cb, void* context)
{
    http_route_context* route = linq_netw_malloc(sizeof(http_route_context));
    linq_netw_assert(route);
    route->cb = cb;
    route->context = context;
    routes_map_add(http->routes, path, &route);
}

void
http_parse_query_str(
    http_route_context* c,
    const char* want,
    const char** result,
    uint32_t* l)
{
    struct mg_str* q = &c->curr_message->query_string;
    if (q) {
        struct mg_str needle = { .p = want, .len = strlen(want) };
        const char *end, *spot = mg_strstr(*q, needle);
        uint32_t wantlen = strlen(want);
        if (spot) {
            if (!(wantlen <= q->len &&                    // bad query str
                  (size_t)spot - (size_t)q->p < q->len && // bad query str
                  (!(spot[wantlen] == '='))))             // bad query str
            {
                spot += wantlen + 1;
                end = memchr(spot, '&', q->len - (spot - q->p));
                *l = end ? end - spot : q->len - (spot - q->p);
                *result = spot;
            } else {
                log_error("(HTTP) Invalid Query String Detected");
                *result = NULL;
                *l = 0;
            }
        } else {
            *result = NULL;
            *l = 0;
        }
    }
}

void
http_printf_json(
    struct mg_connection* connection,
    int code,
    const char* fmt,
    ...)
{
    size_t l;
    va_list ap;

    // Send data
    va_get_len(ap, fmt, l);
    va_start(ap, fmt);
    c_vprintf(connection, code, "application/json", l, fmt, ap);
    va_end(ap);
}

void
http_printf(
    struct mg_connection* connection,
    int code,
    const char* type,
    const char* fmt,
    ...)
{
    size_t l;
    va_list ap;

    // Send data
    va_get_len(ap, fmt, l);
    va_start(ap, fmt);
    c_vprintf(connection, code, type, l, fmt, ap);
    va_end(ap);
}
