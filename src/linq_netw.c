// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_netw.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

#if WITH_MONGOOSE
#include "http.h"
#endif

// Main class
typedef struct linq_netw_s
{
    void* context;
    const linq_netw_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
#if WITH_MONGOOSE
    http_s http;
#endif
} linq_netw_s;

static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    log_error("%06s %04s [%d]", "(ZMTP)", "Event error", e);
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, what, serial);
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* serial, device_s** d)
{
    log_info("%06s %04s [%s]", "(ZMTP)", "Event heartbeat", serial);
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->hb) {
        l->callbacks->hb(l->context, serial, d);
    }
}

static void
on_zmtp_alert(
    void* ctx,
    linq_netw_alert_s* alert,
    linq_netw_email_s* email,
    device_s** d)
{
    log_info(
        "%06s %04s [%s] [%s] [%s] [%s]",
        "(ZMTP)",
        "Event",
        alert->who,
        alert->what,
        alert->where,
        alert->mesg);
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->alert) {
        l->callbacks->alert(l->context, alert, email, d);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
    log_info("%06s %04s", "Received shutdown signal");
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->ctrlc) l->callbacks->ctrlc(l->context);
}

static zmtp_callbacks_s zmtp_callbacks = {
    .err = on_zmtp_error,
    .hb = on_zmtp_heartbeat,
    .alert = on_zmtp_alert,
    .ctrlc = on_zmtp_ctrlc,
};

// Create main context for the caller
linq_netw_s*
linq_netw_create(const linq_netw_callbacks* cb, void* context)
{
    linq_netw_s* l = linq_netw_malloc(sizeof(linq_netw_s));
    if (l) {
        l->devices = device_map_create();
        l->nodes = node_map_create();
        l->callbacks = cb;
        l->context = context ? context : l;
        zmtp_init(&l->zmtp, &l->devices, &l->nodes, &zmtp_callbacks, l);
#if WITH_MONGOOSE
        http_init(&l->http);
#endif
    }
    return l;
}

// Free main context after use
void
linq_netw_destroy(linq_netw_s** linq_netw_p)
{
    linq_netw_s* l = *linq_netw_p;
    *linq_netw_p = NULL;
    zmtp_deinit(&l->zmtp);
    device_map_destroy(&l->devices);
    node_map_destroy(&l->nodes);
#if WITH_MONGOOSE
    http_deinit(&l->http);
#endif
    linq_netw_free(l);
}

void
linq_netw_context_set(linq_netw_s* linq, void* ctx)
{
    linq->context = ctx;
}

// Listen for incoming device connections on "endpoint"
linq_netw_socket
linq_netw_listen(linq_netw_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    if (ep_len > 9 && !(memcmp(ep, "http://*:", 9))) {
        log_info("%06s %04s [%s]", "(HTTP)", "Listening", &ep[9]);
        http_listen(&l->http, &ep[9]);
        return 0;
    } else if (ep_len > 15 && !(memcmp(ep, "http://0.0.0.0:", 15))) {
        log_info("%06s %04s [%s]", "(HTTP)", "Listening", &ep[15]);
        http_listen(&l->http, &ep[15]);
        return 0;
    } else if (ep_len > 17 && !(memcmp(ep, "http://127.0.0.1:", 17))) {
        log_info("%06s %04s [%s]", "(HTTP)", "Listening", &ep[17]);
        http_listen(&l->http, &ep[17]);
        return 0;
    } else {
        log_info("%06s %04s [%s]", "(ZMTP)", "Listening", ep);
        return zmtp_listen(&l->zmtp, ep);
    }
}

// connect to a remote linq and send hello frames
linq_netw_socket
linq_netw_connect(linq_netw_s* l, const char* ep)
{
    return zmtp_connect(&l->zmtp, ep);
}

E_LINQ_ERROR
linq_netw_close_router(linq_netw_s* l, linq_netw_socket handle)
{
    return zmtp_close_router(&l->zmtp, handle);
}

E_LINQ_ERROR
linq_netw_close_dealer(linq_netw_s* l, linq_netw_socket handle)
{
    return zmtp_close_dealer(&l->zmtp, handle);
}

E_LINQ_ERROR
linq_netw_close_http(linq_netw_s* l, linq_netw_socket sock)
{
    ((void)l);
    ((void)sock);
    return LINQ_ERROR_OK;
}

// poll network socket file handles
E_LINQ_ERROR
linq_netw_poll(linq_netw_s* l, int32_t ms)
{
    E_LINQ_ERROR err = zmtp_poll(&l->zmtp, ms);
    if (err) log_error("%06s %04s", "(ZMTP)", "Poll err", err);
#if WITH_MONGOOSE
    err = http_poll(&l->http, ms);
    if (err) { err = LINQ_ERROR_IO; }
#endif
    return err;
}

// get a device from the device map
device_s**
linq_netw_device(const linq_netw_s* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_netw_device_count(const linq_netw_s* l)
{
    return device_map_size(l->devices);
}

// Context used for linq_netw_devices_foreach HOF (Higher Order Function)
typedef struct
{
    const linq_netw_s* l;
    linq_netw_devices_foreach_fn fn;
    void* ctx;
} foreach_device_print_sid_ctx;

// linq_netw_device_foreach HOF
static void
foreach_device_print_sid(device_map_s* self, void* ctx, device_s** d_p)
{
    ((void)self);
    foreach_device_print_sid_ctx* foreach_ctx = ctx;
    foreach_ctx->fn(foreach_ctx->ctx, device_serial(*d_p), device_type(*d_p));
}

// Print a list of serial numbers to caller
void
linq_netw_devices_foreach(
    const linq_netw_s* l,
    linq_netw_devices_foreach_fn fn,
    void* ctx)
{
    foreach_device_print_sid_ctx foreach_ctx = { l,
                                                 fn,
                                                 ctx ? ctx : l->context };
    device_map_foreach(l->devices, foreach_device_print_sid, &foreach_ctx);
}

// return how many nodes are connected to linq
uint32_t
linq_netw_nodes_count(const linq_netw_s* l)
{
    return node_map_size(l->nodes);
}

// send a get request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_get(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_get(&linq->zmtp, serial, path, fn, context);
}

// send a post request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_post(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    linq_netw_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_post(&linq->zmtp, serial, path, json, fn, context);
}

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_delete(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_delete(&linq->zmtp, serial, path, fn, context);
}
