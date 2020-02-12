// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_network.h"
#include "jsmn/jsmn_helpers.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

#if WITH_SQLITE
#include "database/database.h"
#include "http.h"
#include "routes/routes.h"
#endif

#define WEBSOCKET_NEW_FMT                                                      \
    "{"                                                                        \
    "\"type\":\"new\","                                                        \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\","                                                        \
    "\"product\":\"%.*s\","                                                    \
    "\"prjVersion\":\"%.*s\","                                                 \
    "\"atxVersion\":\"%.*s\""                                                  \
    "}}"

#define WEBSOCKET_HEARTBEAT_FMT                                                \
    "{"                                                                        \
    "\"type\":\"heartbeat\","                                                  \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\""                                                         \
    "}}"

#define WEBSOCKET_ALERT_FMT                                                    \
    "{"                                                                        \
    "\"type\":\"alert\","                                                      \
    "\"data\":{"                                                               \
    "\"sid\":\"%.*s\","                                                        \
    "\"who\":\"%.*s\","                                                        \
    "\"what\":\"%.*s\","                                                       \
    "\"siteId\":\"%.*s\","                                                     \
    "\"when\":%d,"                                                             \
    "\"mesg\":\"%.*s\""                                                        \
    "}}"

// Main class
typedef struct linq_network_s
{
    void* context;
    const linq_network_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
#if WITH_SQLITE
    http_s http;
    database_s database;
#endif
} linq_network_s;

static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    log_error("(ZMTP) Event Error [%d]", e);
    linq_network_s* l = ctx;
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, what, serial);
    }
}

static void
on_heartbeat_response(void* ctx, E_LINQ_ERROR e, const char* r, device_s** d)
{
    linq_network_s* l = ctx;
    const char* serial = device_serial(*d);
    if (e) {
        log_warn("(ZMTP) [%.6s...] (%.3d) About request failed!", serial, e);
    } else {
#ifdef WITH_SQLITE
        database_insert_device_from_json(&l->database, serial, r, strlen(r));
#endif
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* sid, device_s** d)
{
    // Note - All tests load devices into context by pushing in heartbeats.
    // Therefore tests should also flush out the response, or mock database
    // query to make tests think device doesn't need to be added into database
    // so there will be no request and response to flush through
    linq_network_s* l = ctx;
#ifdef WITH_SQLITE
    http_broadcast_json(
        &l->http, 200, WEBSOCKET_HEARTBEAT_FMT, strlen(sid), sid);
    if (!database_row_exists_str(&l->database, "devices", "device_id", sid)) {
        log_info(
            "(ZMTP) [%.6s...] "
            "New device connected, requesting about data...",
            device_serial(*d));
        device_send_get(*d, "/ATX/about", on_heartbeat_response, l);
    } else {
        log_debug("(ZMTP) [%.6s...] Heartbeat", device_serial(*d));
    }
#endif
    if (l->callbacks && l->callbacks->hb) {
        l->callbacks->hb(l->context, sid, d);
    }
}

static void
on_zmtp_alert(
    void* ctx,
    linq_network_alert_s* a,
    linq_network_email_s* email,
    device_s** d)
{
    int err;
    const char* serial = device_serial(*d);
    linq_network_s* l = ctx;
    log_info("(ZMTP) [%.6s...] Event Alert", serial);
#ifdef WITH_SQLITE
    // Print out time
    char when[32];
    snprintf(when, sizeof(when), "%.*s", a->when.len, a->when.p);

    // clang-format off
    http_broadcast_json(
        &l->http,
        200,
        WEBSOCKET_ALERT_FMT,
        strlen(serial), serial,
        a->who.len,     a->who.p,
        a->what.len,    a->what.p,
        a->where.len,   a->where.p,
        atoi(when),
        a->mesg.len,    a->mesg.p);
    // clang-format on
    err = database_insert_alert(&l->database, serial, a);
#endif
    log_debug("(ZMTP) Database alert insert result (%d)", err);
    if (l->callbacks && l->callbacks->alert) {
        l->callbacks->alert(l->context, a, email, d);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
    log_info("(ZMTP) Received shutdown signal...");
    linq_network_s* l = ctx;
    if (l->callbacks && l->callbacks->ctrlc) l->callbacks->ctrlc(l->context);
}

static zmtp_callbacks_s zmtp_callbacks = {
    .err = on_zmtp_error,
    .hb = on_zmtp_heartbeat,
    .alert = on_zmtp_alert,
    .ctrlc = on_zmtp_ctrlc,
};

// Create main context for the caller
linq_network_s*
linq_network_create(const linq_network_callbacks* cb, void* context)
{
    linq_network_s* l = linq_network_malloc(sizeof(linq_network_s));
    if (l) {
        l->devices = device_map_create();
        l->nodes = node_map_create();
        l->callbacks = cb;
        l->context = context ? context : l;
        zmtp_init(&l->zmtp, &l->devices, &l->nodes, &zmtp_callbacks, l);
#if WITH_SQLITE
#define ADD_ROUTE(linq, path, fn, ctx) http_use(&(linq)->http, path, fn, ctx)
        database_init(&l->database);
        http_init(&l->http, &l->database);
        ADD_ROUTE(l, "/api/v1/public/create_admin", route_create_admin, l);
        ADD_ROUTE(l, "/api/v1/public/login", route_login, l);
        ADD_ROUTE(l, "/api/v1/login", route_login, l);
        ADD_ROUTE(l, "/api/v1/users", route_users, l);
        ADD_ROUTE(l, "/api/v1/devices", route_devices, l);
        ADD_ROUTE(l, "/api/v1/alerts", route_alerts, l);
        ADD_ROUTE(l, "/api/v1/proxy/...", route_proxy, l);
#endif
    }
    return l;
}

// Free main context after use
void
linq_network_destroy(linq_network_s** linq_network_p)
{
    linq_network_s* l = *linq_network_p;
    *linq_network_p = NULL;
    device_map_destroy(&l->devices);
    node_map_destroy(&l->nodes);
    zmtp_deinit(&l->zmtp);
#if WITH_SQLITE
    http_deinit(&l->http);
    database_deinit(&l->database);
#endif
    linq_network_free(l);
}

#ifdef WITH_SQLITE
database_s*
linq_network_database(linq_network_s* l)
{
    return &l->database;
}
#endif

void
linq_network_context_set(linq_network_s* linq, void* ctx)
{
    linq->context = ctx;
}

// Listen for incoming device connections on "endpoint"
linq_network_socket
linq_network_listen(linq_network_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    if (ep_len > 4 && !memcmp(ep, "http", 4)) {
#ifdef WITH_SQLITE
        if (ep_len > 9 && !(memcmp(ep, "http://*:", 9))) {
            log_info("(HTTP) Listening... [%s]", &ep[9]);
            http_listen(&l->http, &ep[9]);
            return 0;
        } else if (ep_len > 15 && !(memcmp(ep, "http://0.0.0.0:", 15))) {
            log_info("(HTTP) Listening... [%s]", &ep[15]);
            http_listen(&l->http, &ep[15]);
            return 0;
        } else if (ep_len > 17 && !(memcmp(ep, "http://127.0.0.1:", 17))) {
            log_info("(HTTP) Listening... [%s]", &ep[17]);
            http_listen(&l->http, &ep[17]);
            return 0;
        } else {
            return LINQ_ERROR_SOCKET;
        }
#endif
    } else {
        log_info("(ZMTP) Listening... [%s]", ep);
        return zmtp_listen(&l->zmtp, ep);
    }
}

// connect to a remote linq and send hello frames
linq_network_socket
linq_network_connect(linq_network_s* l, const char* ep)
{
    return zmtp_connect(&l->zmtp, ep);
}

void
linq_network_serve(linq_network_s* l, const char* path)
{
#ifdef WITH_SQLITE
    http_serve(&l->http, path);
#endif
}

E_LINQ_ERROR
linq_network_close(linq_network_s* l, linq_network_socket handle)
{
    E_LINQ_ERROR e = LINQ_ERROR_OK;
    if (ATX_NET_SOCKET_TYPE_IS_ROUTER(handle)) {
        zmtp_close_router(&l->zmtp, handle);
    } else if (ATX_NET_SOCKET_TYPE_IS_DEALER(handle)) {
        zmtp_close_dealer(&l->zmtp, handle);
    } else if (ATX_NET_SOCKET_TYPE_IS_HTTP(handle)) {
        // TODO
    } else {
        e = LINQ_ERROR_SOCKET;
    }
    return e;
}

// poll network socket file handles
E_LINQ_ERROR
linq_network_poll(linq_network_s* l, int32_t ms)
{
    E_LINQ_ERROR err = zmtp_poll(&l->zmtp, ms);
    if (err) log_error("(ZMTP) polling error %d", err);
#if WITH_SQLITE
    err = http_poll(&l->http, ms);
    if (err) { err = LINQ_ERROR_IO; }
#endif
    return err;
}

// get a device from the device map
device_s**
linq_network_device(const linq_network_s* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_network_device_count(const linq_network_s* l)
{
    return device_map_size(l->devices);
}

// Context used for linq_network_devices_foreach HOF (Higher Order Function)
typedef struct
{
    const linq_network_s* l;
    linq_network_devices_foreach_fn fn;
    void* ctx;
} foreach_device_print_sid_ctx;

// linq_network_device_foreach HOF
static void
foreach_device_print_sid(
    device_map_s* self,
    void* ctx,
    const char* serial,
    device_s** d_p)
{
    ((void)self);
    foreach_device_print_sid_ctx* foreach_ctx = ctx;
    foreach_ctx->fn(foreach_ctx->ctx, serial, device_type(*d_p));
}

// Print a list of serial numbers to caller
void
linq_network_devices_foreach(
    const linq_network_s* l,
    linq_network_devices_foreach_fn fn,
    void* ctx)
{
    foreach_device_print_sid_ctx foreach_ctx = { l,
                                                 fn,
                                                 ctx ? ctx : l->context };
    device_map_foreach(l->devices, foreach_device_print_sid, &foreach_ctx);
}

// return how many nodes are connected to linq
uint32_t
linq_network_node_count(const linq_network_s* l)
{
    return node_map_size(l->nodes);
}

// send a get request to a device connected to us
E_LINQ_ERROR
linq_network_send_get(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_get(&linq->zmtp, serial, path, fn, context);
}

// send a post request to a device connected to us
E_LINQ_ERROR
linq_network_send_post(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_post(&linq->zmtp, serial, path, json, fn, context);
}

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_network_send_delete(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_delete(&linq->zmtp, serial, path, fn, context);
}
