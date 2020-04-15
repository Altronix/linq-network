// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "linq_network.h"
#include "jsmn_helpers.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

// Main class
typedef struct linq_network_s
{
    void* context;
    const linq_network_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
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
on_zmtp_heartbeat(void* ctx, const char* sid, device_s** d)
{
    linq_network_s* l = ctx;
    log_info("(ZMTP) [%.6s...] Event Alert", sid);
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
    const char* serial = device_serial(*d);
    linq_network_s* l = ctx;
    log_info("(ZMTP) [%.6s...] Event Alert", serial);
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
    linq_network_free(l);
}

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
    log_info("(ZMTP) Listening... [%s]", ep);
    return zmtp_listen(&l->zmtp, ep);
}

// connect to a remote linq and send hello frames
linq_network_socket
linq_network_connect(linq_network_s* l, const char* ep)
{
    return zmtp_connect(&l->zmtp, ep);
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
E_LINQ_ERROR
linq_network_send_get_mem(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    uint32_t plen,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_get_mem(
        &linq->zmtp, serial, path, plen, fn, context);
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

E_LINQ_ERROR
linq_network_send_post_mem(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_post_mem(
        &linq->zmtp, serial, path, plen, json, jlen, fn, context);
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

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_network_send_delete_mem(
    const linq_network_s* linq,
    const char* serial,
    const char* path,
    uint32_t plen,
    linq_network_request_complete_fn fn,
    void* context)
{
    return zmtp_device_send_delete_mem(
        &linq->zmtp, serial, path, plen, fn, context);
}
