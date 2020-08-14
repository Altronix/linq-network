// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netw.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

// Main class
typedef struct netw_s
{
    void* context;
    const netw_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
} netw_s;

static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    log_error("(ZMTP) Event Error [%d]", e);
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_err) {
        l->callbacks->on_err(l->context, e, what, serial);
    }
}

static void
on_zmtp_new(void* ctx, const char* sid)
{
    netw_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event New Device", sid);
    if (l->callbacks && l->callbacks->on_new) {
        l->callbacks->on_new(l->context, sid);
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* sid)
{
    netw_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event Heartbeat", sid);
    if (l->callbacks && l->callbacks->on_heartbeat) {
        l->callbacks->on_heartbeat(l->context, sid);
    }
}

static void
on_zmtp_alert(
    void* ctx,
    const char* serial,
    netw_alert_s* a,
    netw_email_s* email)
{
    netw_s* l = ctx;
    log_debug("(ZMTP) [%.6s...] Event Alert", serial);
    if (l->callbacks && l->callbacks->on_alert) {
        l->callbacks->on_alert(l->context, serial, a, email);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
    log_info("(ZMTP) Received shutdown signal...");
    netw_s* l = ctx;
    if (l->callbacks && l->callbacks->on_ctrlc) {
        l->callbacks->on_ctrlc(l->context);
    }
}

static netw_callbacks zmtp_callbacks = {
    .on_err = on_zmtp_error,
    .on_new = on_zmtp_new,
    .on_heartbeat = on_zmtp_heartbeat,
    .on_alert = on_zmtp_alert,
    .on_ctrlc = on_zmtp_ctrlc,
};

// Create main context for the caller
netw_s*
netw_create(const netw_callbacks* cb, void* context)
{
    netw_s* l = linq_network_malloc(sizeof(netw_s));
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
netw_destroy(netw_s** netw_p)
{
    netw_s* l = *netw_p;
    *netw_p = NULL;
    device_map_destroy(&l->devices);
    node_map_destroy(&l->nodes);
    zmtp_deinit(&l->zmtp);
    linq_network_free(l);
}

void
netw_context_set(netw_s* linq, void* ctx)
{
    linq->context = ctx;
}

// Listen for incoming device connections on "endpoint"
netw_socket
netw_listen(netw_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    log_info("(ZMTP) Listening... [%s]", ep);
    return zmtp_listen(&l->zmtp, ep);
}

// connect to a remote linq and send hello frames
netw_socket
netw_connect(netw_s* l, const char* ep)
{
    return zmtp_connect(&l->zmtp, ep);
}

E_LINQ_ERROR
netw_close(netw_s* l, netw_socket handle)
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
netw_poll(netw_s* l, int32_t ms)
{
    E_LINQ_ERROR err = zmtp_poll(&l->zmtp, ms);
    if (err) log_error("(ZMTP) polling error %d", err);

    // Loop through devices
    device_map_foreach_poll(l->devices);

    return err;
}

// get a device from the device map
node_s**
netw_device(const netw_s* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// Check if the serial number is known in our hash table
bool
netw_device_exists(const netw_s* linq, const char* sid)
{
    return device_map_get(linq->devices, sid) ? true : false;
}

// return how many devices are connected to linq
uint32_t
netw_device_count(const netw_s* l)
{
    return device_map_size(l->devices);
}

// // Context used for netw_devices_foreach HOF (Higher Order Function)
// typedef struct
// {
//     const netw_s* l;
//     netw_devices_foreach_fn fn;
//     void* ctx;
// } foreach_device_print_sid_ctx;
//
// // netw_device_foreach HOF
// static void
// foreach_device_print_sid(
//     device_map_s* self,
//     void* ctx,
//     const char* serial,
//     device_zmtp_s** d_p)
// {
//     ((void)self);
//     foreach_device_print_sid_ctx* foreach_ctx = ctx;
//     foreach_ctx->fn(foreach_ctx->ctx, serial, device_type(*d_p));
// }
//
// // Print a list of serial numbers to caller
// void
// netw_devices_foreach(const netw_s* l, netw_devices_foreach_fn fn, void* ctx)
// {
//     foreach_device_print_sid_ctx foreach_ctx = { l,
//                                                  fn,
//                                                  ctx ? ctx : l->context };
//     device_map_foreach(l->devices, foreach_device_print_sid, &foreach_ctx);
// }

// return how many nodes are connected to linq
uint32_t
netw_node_count(const netw_s* l)
{
    return node_map_size(l->nodes);
}

static E_REQUEST_METHOD
method_from_str(const char* method)
{
    uint32_t l = strlen(method);
    if (l == 3) {
        if (!memcmp(method, "GET", l)) {
            return REQUEST_METHOD_GET;
        } else if (!(memcmp(method, "PUT", l))) {
            return REQUEST_METHOD_POST; // TODO support PUT
        }
    } else if (l == 4 && !memcmp(method, "POST", l)) {
        return REQUEST_METHOD_POST;
    } else if (l == 6 && !memcmp(method, "DELETE", l)) {
        return REQUEST_METHOD_DELETE;
    }
    assert(false);
    return -1; // should never return
}

// send a get request to a device connected to us
E_LINQ_ERROR
netw_send(
    const netw_s* linq,
    const char* serial,
    const char* method,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_request_complete_fn fn,
    void* context)
{
    E_LINQ_ERROR error = LINQ_ERROR_OK;
    char e[32];
    node_s** node = device_map_get(linq->devices, serial);
    if (!node) {
        snprintf(e, sizeof(e), "{\"error\":%d}", LINQ_ERROR_DEVICE_NOT_FOUND);
        error = LINQ_ERROR_DEVICE_NOT_FOUND;
        fn(context, "", error, e);
    } else {
        E_REQUEST_METHOD m = method_from_str(method);
        (*node)->send(*node, m, path, plen, json, jlen, fn, context);
    }
    return error;
}
