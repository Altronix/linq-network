// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/atx_net.h"
#include "jsmn_helpers.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

#if WITH_SQLITE
#include "database/database.h"
#include "http.h"
#include "routes/routes.h"
#endif

// Main class
typedef struct atx_net_s
{
    void* context;
    const atx_net_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
#if WITH_SQLITE
    http_s http;
    database_s database;
#endif
} atx_net_s;

static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    log_error("(ZMTP) Event Error [%d]", e);
    atx_net_s* l = ctx;
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, what, serial);
    }
}

static void
on_heartbeat_response(
    void* ctx,
    E_LINQ_ERROR e,
    const char* response,
    device_s** d)
{
    atx_net_s* l = ctx;
    atx_str sid, product, prj_version, atx_version, web_version, mac;
    const char* ser = device_serial(*d);
    uint32_t count;
    jsmntok_t t[64];
    if (e) {
        log_warn("(ZMTP) [%.6s...] (%.3d) About request failed!", ser, e);
    } else {
#ifdef WITH_SQLITE
        char keys[128], vals[128];
        // clang-format off
        log_info(
            "(ZMTP) [%.6s...] (%.3d) Adding device to database...",
            ser,
            e);
        count = jsmn_parse_tokens_path(
            "/about",
            t,
            64,
            response,
            strlen(response),
            6,
            "sid",        &sid,
            "product",    &product,
            "prjVersion", &prj_version,
            "atxVersion", &atx_version,
            "webVersion", &web_version,
            "mac",        &mac);
        if(count == 6){
            uint32_t keylen = snprintf(
                keys, sizeof(keys),
                "%s",
                "device_id,product,prj_version,atx_version,web_version,mac");
            uint32_t vallen = snprintf(
                vals, sizeof(vals),
                "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\"",
                sid.len,         sid.p,
                product.len,     product.p,
                prj_version.len, prj_version.p,
                atx_version.len, atx_version.p,
                web_version.len, web_version.p,
                mac.len,         mac.p);
            // clang-format on
            int err = database_insert_raw_n(
                &l->database, "devices", keys, keylen, vals, vallen);
            log_debug("(ZMTP) Database device insert result (%d)", err);
        } else {
            log_debug("(ZMTP) Heartbeat parser error");
        }
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
    atx_net_s* l = ctx;
#ifdef WITH_SQLITE
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
    atx_net_alert_s* a,
    atx_net_email_s* email,
    device_s** d)
{
    int err;
    char k[128], v[256];
    uint32_t klen, vlen;
    const char* serial = device_serial(*d);
    atx_net_s* l = ctx;
    zuuid_t* uid = zuuid_new();
    atx_net_assert(uid);
    log_info("(ZMTP) [%.6s...] Event Alert", serial);
    klen =
        snprintf(k, sizeof(k), "alert_id,who,what,site_id,time,mesg,device_id");
    // clang-format off
    vlen = snprintf(
        v,
        sizeof(v),
        "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%.*s,\"%.*s\",\"%.*s\"",
        32,                  zuuid_str(uid),
        a->who.len,          a->who.p,
        a->what.len,         a->what.p,
        a->where.len,        a->where.p,
        a->when.len,         a->when.p,
        a->mesg.len,         a->mesg.p,
        (int)strlen(serial), serial);
    // clang-format on
    zuuid_destroy(&uid);
#ifdef WITH_SQLITE
    err = database_insert_raw_n(&l->database, "alerts", k, klen, v, vlen);
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
    atx_net_s* l = ctx;
    if (l->callbacks && l->callbacks->ctrlc) l->callbacks->ctrlc(l->context);
}

static zmtp_callbacks_s zmtp_callbacks = {
    .err = on_zmtp_error,
    .hb = on_zmtp_heartbeat,
    .alert = on_zmtp_alert,
    .ctrlc = on_zmtp_ctrlc,
};

// Create main context for the caller
atx_net_s*
atx_net_create(const atx_net_callbacks* cb, void* context)
{
    atx_net_s* l = atx_net_malloc(sizeof(atx_net_s));
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
atx_net_destroy(atx_net_s** atx_net_p)
{
    atx_net_s* l = *atx_net_p;
    *atx_net_p = NULL;
    device_map_destroy(&l->devices);
    node_map_destroy(&l->nodes);
    zmtp_deinit(&l->zmtp);
#if WITH_SQLITE
    http_deinit(&l->http);
    database_deinit(&l->database);
#endif
    atx_net_free(l);
}

#ifdef WITH_SQLITE
database_s*
atx_net_database(atx_net_s* l)
{
    return &l->database;
}
#endif

void
atx_net_context_set(atx_net_s* linq, void* ctx)
{
    linq->context = ctx;
}

// Listen for incoming device connections on "endpoint"
atx_net_socket
atx_net_listen(atx_net_s* l, const char* ep)
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
atx_net_socket
atx_net_connect(atx_net_s* l, const char* ep)
{
    return zmtp_connect(&l->zmtp, ep);
}

E_LINQ_ERROR
atx_net_close(atx_net_s* l, atx_net_socket handle)
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
atx_net_poll(atx_net_s* l, int32_t ms)
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
atx_net_device(const atx_net_s* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
atx_net_device_count(const atx_net_s* l)
{
    return device_map_size(l->devices);
}

// Context used for atx_net_devices_foreach HOF (Higher Order Function)
typedef struct
{
    const atx_net_s* l;
    atx_net_devices_foreach_fn fn;
    void* ctx;
} foreach_device_print_sid_ctx;

// atx_net_device_foreach HOF
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
atx_net_devices_foreach(
    const atx_net_s* l,
    atx_net_devices_foreach_fn fn,
    void* ctx)
{
    foreach_device_print_sid_ctx foreach_ctx = { l,
                                                 fn,
                                                 ctx ? ctx : l->context };
    device_map_foreach(l->devices, foreach_device_print_sid, &foreach_ctx);
}

// return how many nodes are connected to linq
uint32_t
atx_net_node_count(const atx_net_s* l)
{
    return node_map_size(l->nodes);
}

// send a get request to a device connected to us
E_LINQ_ERROR
atx_net_send_get(
    const atx_net_s* linq,
    const char* serial,
    const char* path,
    atx_net_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_get(&linq->zmtp, serial, path, fn, context);
}

// send a post request to a device connected to us
E_LINQ_ERROR
atx_net_send_post(
    const atx_net_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    atx_net_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_post(&linq->zmtp, serial, path, json, fn, context);
}

// send a delete request to a device connected to us
E_LINQ_ERROR
atx_net_send_delete(
    const atx_net_s* linq,
    const char* serial,
    const char* path,
    atx_net_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_delete(&linq->zmtp, serial, path, fn, context);
}
