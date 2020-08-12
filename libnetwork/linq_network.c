// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "linq_network.h"
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
    if (l->callbacks && l->callbacks->on_err) {
        l->callbacks->on_err(l->context, e, what, serial);
    }
}

static void
on_zmtp_new(void* ctx, const char* sid)
{
    linq_network_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event New Device", sid);
    if (l->callbacks && l->callbacks->on_new) {
        l->callbacks->on_new(l->context, sid);
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* sid)
{
    linq_network_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event Heartbeat", sid);
    if (l->callbacks && l->callbacks->on_heartbeat) {
        l->callbacks->on_heartbeat(l->context, sid);
    }
}

static void
on_zmtp_alert(
    void* ctx,
    const char* serial,
    linq_network_alert_s* a,
    linq_network_email_s* email)
{
    linq_network_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event Alert", serial);
    if (l->callbacks && l->callbacks->on_alert) {
        l->callbacks->on_alert(l->context, serial, a, email);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
    log_info("(ZMTP) Received shutdown signal...");
    linq_network_s* l = ctx;
    if (l->callbacks && l->callbacks->on_ctrlc) {
        l->callbacks->on_ctrlc(l->context);
    }
}

static linq_network_callbacks zmtp_callbacks = {
    .on_err = on_zmtp_error,
    .on_new = on_zmtp_new,
    .on_heartbeat = on_zmtp_heartbeat,
    .on_alert = on_zmtp_alert,
    .on_ctrlc = on_zmtp_ctrlc,
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
void**
linq_network_device(const linq_network_s* l, const char* serial)
{
    return (void**)device_map_get(l->devices, serial);
}

// Check if the serial number is known in our hash table
bool
linq_network_device_exists(const linq_network_s* linq, const char* sid)
{
    return device_map_get(linq->devices, sid) ? true : false;
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
linq_network_send(
    const linq_network_s* linq,
    const char* serial,
    const char* method,
    const char* path,
    uint32_t path_len,
    const char* json,
    uint32_t json_len,
    linq_network_request_complete_fn callback,
    void* context)
{
    return zmtp_device_send(
        &linq->zmtp,
        serial,
        method,
        path,
        path_len,
        json,
        json_len,
        callback,
        context);
}
