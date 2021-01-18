// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "http.h"
#include "http_auth.h"
#include "http_users.h"
#include "log.h"

#define UNSECURE_API "/api/v1/public"
#define UNSECURE_API_LEN (sizeof(UNSECURE_API) - 1)

#define http_info(...) log_info("HTTP", __VA_ARGS__)
#define http_warn(...) log_warn("HTTP", __VA_ARGS__)
#define http_debug(...) log_debug("HTTP", __VA_ARGS__)
#define http_trace(...) log_trace("HTTP", __VA_ARGS__)
#define http_error(...) log_error("HTTP", __VA_ARGS__)
#define http_fatal(...) log_fatal("HTTP", __VA_ARGS__)

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

// Hash map of URL's to routes and requests
MAP_INIT_W_FREE(routes, http_route_context);
MAP_INIT_W_FREE(requests, http_request_s);

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

static http_route_context**
resolve_route(http_s* http, struct mg_str* uri)
{
    static char path[128];
    routes_iter iter;
    linq_network_assert(uri->len < sizeof(path));
    http_route_context** r_p = NULL;
    snprintf(path, sizeof(path), "%.*s", (uint32_t)uri->len, uri->p);
    r_p = routes_map_get(http->routes, path);
    if (!r_p) {
        map_foreach(http->routes, iter)
        {
            if (map_has_key(http->routes, iter)) {
                const char* key = map_key(http->routes, iter);
                uint32_t tlen = strlen(key), plen = strlen(path);
                if (tlen >= 4 &&                          //
                    plen > tlen &&                        //
                    !memcmp(&key[tlen - 4], "/...", 4) && //
                    !memcmp(key, path, tlen - 4)) {
                    r_p = &map_val(http->routes, iter);
                }
            }
        }
    }
    return r_p;
}

static void
process_request(
    http_s* http,
    http_route_context** route_p,
    struct mg_connection* c,
    struct http_message* m)
{
    static uint32_t reqidx = 0;
    http_request_s* r = malloc(sizeof(http_request_s));
    assert(r);
    r->route_p = route_p;
    r->connection = c;
    r->http = http;
    r->closed = r->more = false;
    (*r->route_p)->curr_message = m;
    (*r->route_p)->cb(r, get_method(m), m->body.len, m->body.p);
    if (r->more) {
        snprintf(r->key, sizeof(r->key), "%d", reqidx++);
        requests_map_add(http->requests, r->key, &r);
    } else {
        free(r);
    }
}

static void
process_file(http_s* http, struct mg_connection* c, struct http_message* m)
{
    char b[512];
    const char* root = http->serve_opts.document_root;
    int err;
    unsigned int _p;
    struct stat st;
    struct mg_str _s, _u, _h, path, _q, _f;
    err = mg_parse_uri(m->uri, &_s, &_u, &_h, &_p, &path, &_q, &_f);
    if (err) return;
    snprintf(b, sizeof(b), "%s%.*s", root, (int)path.len, path.p);
    err = stat(b, &st);
    if (err) {
        http_debug("[%s] not found... Serving root", b);
        snprintf(b, sizeof(b), "%s%s", root, "/index.html");
        http_debug("(root) [%s]", b);
        mg_http_serve_file(c, m, b, mg_mk_str("text/html"), mg_mk_str(""));
    } else {
        http_debug("[%s] found... ", b);
        mg_serve_http(c, m, http->serve_opts);
    }
}

static void
ev_handler(struct mg_connection* c, int ev, void* p, void* user_data)
{
    switch (ev) {
        case MG_EV_POLL: break;
        case MG_EV_ACCEPT: http_trace("%04s", "Accept"); break;
        case MG_EV_CONNECT: http_trace("%04s", "Connect"); break;
        case MG_EV_RECV: http_trace("%04s", "Recv"); break;
        case MG_EV_SEND: http_trace("%04s", "Send"); break;
        case MG_EV_CLOSE: {
            http_s* http = user_data;
            char addr[48];
            requests_iter iter;
            map_foreach(http->requests, iter)
            {
                if (map_has_key(http->requests, iter)) {
                    if (map_val(http->requests, iter)->connection == c) {
                        map_val(http->requests, iter)->closed = true;
                    }
                }
            }
            get_addr(c, addr, sizeof(addr));
            http_info("(%s) Connection close", addr);
        } break;
        case MG_EV_TIMER: http_trace("%04s", "Timer"); break;
        case MG_EV_HTTP_REQUEST: {
            http_s* http = user_data;
            struct http_message* m = (struct http_message*)p;
            struct mg_str* path = &m->uri;
            http_info("serve [%.*s]", path->len, path->p);
            http_route_context** r = resolve_route(http, path);
            if (r) {
                if (path->len >= UNSECURE_API_LEN &&
                    !(memcmp(UNSECURE_API, path->p, UNSECURE_API_LEN))) {
                    process_request(http, r, c, m);
                } else {
                    if (http_auth_is_authorized(http->db, c, m)) {
                        process_request(http, r, c, m);
                    } else {
                        http_warn("%04s %s [%s]", "Req.", "(503)", path);
                        c_printf_json(c, 503, JERROR_503);
                    }
                }
            } else if (http->serve_opts.document_root) {
                process_file(http, c, m);
            } else {
                http_warn("%04s %s [%s]", "Req.", "(404)", path);
                c_printf_json(c, 404, "{\"error\":\"%s\"}", "not found");
            }
        } break;
        case MG_EV_HTTP_REPLY: http_trace("%04s", "Reply"); break;
        case MG_EV_HTTP_CHUNK: http_trace("%04s", "Chunk"); break;
        case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
            char addr[48];
            get_addr(c, addr, sizeof(addr));
            http_info("(%s) Received websocket request...", addr);
        } break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
        case MG_EV_WEBSOCKET_FRAME: http_trace("%04s", "Websocket frame");
        case MG_EV_WEBSOCKET_CONTROL_FRAME:
            http_trace("%04s", "Websocket ctrl frame");
            break;
        case MG_EV_HTTP_MULTIPART_REQUEST:
        case MG_EV_HTTP_PART_BEGIN:
        case MG_EV_HTTP_PART_DATA:
        case MG_EV_HTTP_PART_END:
        case MG_EV_HTTP_MULTIPART_REQUEST_END:
            http_trace("%04s", "Recv");
            break;
        default: http_error("%04s %s (%d)", "Recv", "Unkown", ev); break;
    }
}

void
http_init(http_s* http, database_s* db)
{
    memset(http, 0, sizeof(http_s));
    http->db = db;
    mg_mgr_init(&http->connections, http);
    http->routes = routes_map_create();
    http->requests = requests_map_create();

    http_use(http, "/api/v1/public/create_admin", create_admin, http);
    http_use(http, "/api/v1/public/has_admin", has_admin, http);
    http_use(http, "/api/v1/public/login", login, http);
    http_use(http, "/api/v1/users", users, http);
}

void
http_deinit(http_s* http)
{
    mg_mgr_free(&http->connections);
    routes_map_destroy(&http->routes);
    requests_map_destroy(&http->requests);
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
        http_fatal("%10s", "HTTP can only listen to one server at a time!");
        http_fatal("%10s", "Please shutdown HTTP server ");
        linq_network_assert(http->http);
    }
    http_info("Listening... [http://*:%s]", port);
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
        http_fatal("%10s", "HTTP can only listen to one server at a time!");
        http_fatal("%10s", "Please shutdown HTTP server");
        linq_network_assert(http->http);
    }
    memset(&opts, 0, sizeof(opts));
    opts.ssl_cert = cert;
    opts.ssl_key = key;
    opts.error_string = &err;
    opts.user_data = http;
    http->https = mg_bind_opt(&http->connections, port, ev_handler, http, opts);
    if (!(http->https == NULL)) {
        http_info("Listening... [https://*:%s]", port);
        mg_set_protocol_http_websocket(http->https);
    } else {
        http_error("Listening error [%s]", err);
        http_error("CERT: [%s]", cert);
        http_error("KEY: [%s]", key);
        http_error("PORT: [%s]", port);
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
http_root(http_s* http, const char* path)
{
    snprintf(http->root, sizeof(http->root), "%s", path);
    http->serve_opts.document_root = http->root;
    http_info("Serving... [%s]", http->root);
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
                http_error("Invalid Query String Detected");
                *result = NULL;
                *l = 0;
            }
        } else {
            *result = NULL;
            *l = 0;
        }
    }
}

void*
http_request_context(http_request_s* r)
{
    return (*r->route_p)->context;
}

struct mg_connection*
http_request_connection(http_request_s* r)
{
    return r->connection;
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
LINQ_HTTP_EXPORT void
http_resolve_json(http_request_s* r, int code, const char* fmt, ...)
{
    size_t l;
    va_list ap;

    if (!r->closed) {
        // Send data
        va_get_len(ap, fmt, l);
        va_start(ap, fmt);
        c_vprintf(r->connection, code, "application/json", l, fmt, ap);
        va_end(ap);
    }
    requests_map_remove(r->http->requests, r->key);
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
