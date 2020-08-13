// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "http.h"
#include "http_auth.h"
#include "log.h"
#include "routes.h"

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

const char*
http_error_message(E_LINQ_ERROR e)
{
    static const char* ok = "ok";
    static const char* server_error = "server error";
    static const char* client_error = "client error";
    static const char* authorization = "authorization error";
    static const char* not_found = "not found";
    static const char* busy = "try again later";
    int code = http_error_code(e);
    if (code == 200) {
        return ok;
    } else if (code == 400) {
        return client_error;
    } else if (code == 403) {
        return authorization;
    } else if (code == 404) {
        return not_found;
    } else if (code == 504) {
        return busy;
    } else {
        return server_error;
    }
}

uint32_t
http_error_code(E_LINQ_ERROR e)
{
    switch (e) {
        case LINQ_ERROR_OK: return 200;
        case LINQ_ERROR_BAD_ARGS:
        case LINQ_ERROR_PROTOCOL:
        case LINQ_ERROR_400: return 400;
        case LINQ_ERROR_403: return 403;
        case LINQ_ERROR_DEVICE_NOT_FOUND:
        case LINQ_ERROR_404: return 404;
        case LINQ_ERROR_SHUTTING_DOWN:
        case LINQ_ERROR_IO:
        case LINQ_ERROR_OOM:
        case LINQ_ERROR_500: return 500;
        case LINQ_ERROR_TIMEOUT:
        case LINQ_ERROR_504: return 504;
        default: return 500;
    }
}

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

static inline bool
is_websocket(const struct mg_connection* c)
{
    return c->flags & MG_F_IS_WEBSOCKET;
}

static inline void
get_addr(const struct mg_connection* c, char* buff, uint32_t sbuff)
{
    mg_sock_addr_to_str(
        &c->sa, buff, sbuff, MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
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
resolve_route(http_s* http, struct mg_str* uri)
{
    static char path[128];
    linq_network_assert(uri->len < sizeof(path));
    http_route_context** r_p = NULL;
    snprintf(path, sizeof(path), "%.*s", (uint32_t)uri->len, uri->p);
    foreach_route_check_path_context ctx = { .path = path, .found_p = &r_p };
    r_p = routes_map_get(http->routes, path);
    if (!r_p) routes_map_foreach(http->routes, foreach_route_check_path, &ctx);
    return r_p;
}

static void
process_route(
    http_route_context** r,
    struct mg_connection* c,
    struct http_message* m)
{
    (*r)->curr_connection = c;
    (*r)->curr_message = m;
    (*r)->cb(*r, get_method(m), m->body.len, m->body.p);
}

static void
ev_handler(struct mg_connection* c, int ev, void* p, void* user_data)
{
    switch (ev) {
        case MG_EV_POLL: break;
        case MG_EV_ACCEPT: log_trace("%06s %04s", "(HTTP)", "Accept"); break;
        case MG_EV_CONNECT: log_trace("%06s %04s", "(HTTP)", "Connect"); break;
        case MG_EV_RECV: log_trace("%06s %04s", "(HTTP)", "Recv"); break;
        case MG_EV_SEND: log_trace("%06s %04s", "(HTTP)", "Send"); break;
        case MG_EV_CLOSE: {
            char addr[48];
            get_addr(c, addr, sizeof(addr));
            log_info("(HTTP) (%s) Connection close", addr);
        } break;
        case MG_EV_TIMER: log_trace("%06s %04s", "(HTTP)", "Timer"); break;
        case MG_EV_HTTP_REQUEST: {
            http_s* http = user_data;
            struct http_message* m = (struct http_message*)p;
            struct mg_str* path = &m->uri;
            http_route_context** r = resolve_route(http, path);
            if (r) {
                if (path->len >= UNSECURE_API_LEN &&
                    !(memcmp(UNSECURE_API, path->p, UNSECURE_API_LEN))) {
                    process_route(r, c, m);
                } else {
                    if (http_auth_is_authorized(&http->db, c, m)) {
                        process_route(r, c, m);
                    } else {
                        log_warn(
                            "%06s %04s %s [%s]",
                            "(HTTP)",
                            "Req.",
                            "(503)",
                            path);
                        c_printf_json(c, 503, JERROR_503);
                    }
                }
            } else if (http->serve_opts.document_root) {
                mg_serve_http(c, m, http->serve_opts);
            } else {
                log_warn("%06s %04s %s [%s]", "(HTTP)", "Req.", "(404)", path);
                c_printf_json(c, 404, "{\"error\":\"%s\"}", "not found");
            }
        } break;
        case MG_EV_HTTP_REPLY: log_trace("%06s %04s", "(HTTP)", "Reply"); break;
        case MG_EV_HTTP_CHUNK: log_trace("%06s %04s", "(HTTP)", "Chunk"); break;
        case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
            char addr[48];
            get_addr(c, addr, sizeof(addr));
            log_info("(HTTP) (%s) Received websocket request...", addr);
        } break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
        case MG_EV_WEBSOCKET_FRAME:
            log_trace("%06s %04s", "(HTTP)", "Websocket frame");
        case MG_EV_WEBSOCKET_CONTROL_FRAME:
            log_trace("%06s %04s", "(HTTP)", "Websocket ctrl frame");
            break;
        case MG_EV_HTTP_MULTIPART_REQUEST:
        case MG_EV_HTTP_PART_BEGIN:
        case MG_EV_HTTP_PART_DATA:
        case MG_EV_HTTP_PART_END:
        case MG_EV_HTTP_MULTIPART_REQUEST_END:
            log_trace("%06s %04s", "(HTTP)", "Recv");
            break;
        default:
            log_error("%06s %04s %s (%d)", "(HTTP)", "Recv", "Unkown", ev);
            break;
    }
}

void
http_init(http_s* http, netw_s* l)
{
#define ADD_ROUTE(http, path, fn, ctx) http_use(http, path, fn, ctx)
    memset(http, 0, sizeof(http_s));
    http->linq = l;
    database_init(&http->db);
    mg_mgr_init(&http->connections, http);
    http->routes = routes_map_create();
    ADD_ROUTE(http, "/api/v1/public/create_admin", route_create_admin, http);
    ADD_ROUTE(http, "/api/v1/public/login", route_login, http);
    ADD_ROUTE(http, "/api/v1/users", route_users, http);
    ADD_ROUTE(http, "/api/v1/devices", route_devices, http);
    ADD_ROUTE(http, "/api/v1/alerts", route_alerts, http);
    ADD_ROUTE(http, "/api/v1/proxy/...", route_proxy, http);
#undef ADD_ROUTE
}

void
http_deinit(http_s* http)
{
    mg_mgr_free(&http->connections);
    routes_map_destroy(&http->routes);
    database_deinit(&http->db);
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
    if (http->http) {
        log_fatal("%10s", "HTTP can only listen to one server at a time!");
        log_fatal("%10s", "Please shutdown HTTP server before listening again");
        linq_network_assert(http->http);
    }
    log_info("(HTTP) Listening... [http://*:%s]", port);
    http->http = mg_bind(&http->connections, port, ev_handler, http);
    mg_set_protocol_http_websocket(http->http);
}

void
http_listen_tls(
    http_s* http,
    const char* port,
    const char* cert,
    const char* key)
{
    const char* err;
    struct mg_bind_opts opts;
    if (http->https) {
        log_fatal("%10s", "HTTP can only listen to one server at a time!");
        log_fatal("%10s", "Please shutdown HTTP server before listening again");
        linq_network_assert(http->http);
    }
    memset(&opts, 0, sizeof(opts));
    opts.ssl_cert = cert;
    opts.ssl_key = key;
    opts.error_string = &err;
    opts.user_data = http;
    http->https = mg_bind_opt(&http->connections, port, ev_handler, http, opts);
    if (!(http->https == NULL)) {
        log_info("(HTTP) Listening... [https://*:%s]", port);
        mg_set_protocol_http_websocket(http->https);
    } else {
        log_error("(HTTP) Listening error [%s]", err);
        log_error("(HTTP) CERT: [%s]", cert);
        log_error("(HTTP)  KEY: [%s]", key);
        log_error("(HTTP) PORT: [%s]", port);
    }
}

void
http_use(http_s* http, const char* path, http_route_cb cb, void* context)
{
    http_route_context* route = linq_network_malloc(sizeof(http_route_context));
    linq_network_assert(route);
    route->cb = cb;
    route->context = context;
    routes_map_add(http->routes, path, &route);
}

void
http_serve(http_s* http, const char* path)
{
    log_info("(HTTP) Serving... [%s]", path);
    http->serve_opts.document_root = path;
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
http_broadcast_json(http_s* http, int code, const char* fmt, ...)
{
    char mem[1024];
    struct mg_connection* c;
    int len;
    va_list list;

    va_start(list, fmt);
    len = vsnprintf(mem, sizeof(mem), fmt, list);
    va_end(list);

    if (len) {
        // TODO need __wrap_mg_next() to test broadcast output
        for (c = mg_next(&http->connections, NULL); c != NULL;
             c = mg_next(&http->connections, c)) {
            mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, mem, len);
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
