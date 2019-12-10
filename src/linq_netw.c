// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base64.h"
#include "containers.h"
#include "device.h"
#include "linq_netw_internal.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"

#define JSMN_HEADER
#include "jsmn/jsmn.h"

MAP_INIT(socket, zsock_t, zsock_destroy);

// Main class
typedef struct linq_netw_s
{
    void* context;
    const linq_netw_callbacks* callbacks;
    device_map_s* devices;
    node_map_s* nodes;
    zmtp_s zmtp;
} linq_netw_s;

static void
on_zmtp_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, what, serial);
    }
}

static void
on_zmtp_heartbeat(void* ctx, const char* serial, device_s** d)
{
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
    linq_netw_s* l = ctx;
    if (l->callbacks && l->callbacks->alert) {
        l->callbacks->alert(l->context, alert, email, d);
    }
}

static void
on_zmtp_ctrlc(void* ctx)
{
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
    zsock_t* socket = zsock_new_router(ep);
    if (socket) {
        socket_map_add(l->zmtp.routers, ep, &socket);
        return socket_map_key(l->zmtp.routers, ep);
    } else {
        return LINQ_ERROR_SOCKET;
    }
}

// connect to a remote linq and send hello frames
linq_netw_socket
linq_netw_connect(linq_netw_s* l, const char* ep)
{
    zsock_t* socket = zsock_new_dealer(ep);
    if (socket) {
        socket_map_add(l->zmtp.dealers, ep, &socket);
        node_s* n =
            node_create(*socket_map_get(l->zmtp.dealers, ep), NULL, 0, ep);
        if (n) {
            node_send_hello(n);
            node_map_add(l->nodes, ep, &n);
        }
        return socket_map_key(l->zmtp.dealers, ep);
    }
    return LINQ_ERROR_SOCKET;
}

static void
foreach_device_remove_if_sock_eq(
    device_map_s* self,
    void* ctx,
    device_s** device_p)
{
    zsock_t* eq = ctx;
    linq_netw_socket_s* socket = ((linq_netw_socket_s*)*device_p);
    if (eq == socket->sock) device_map_remove(self, device_serial(*device_p));
}

static void
foreach_node_remove_if_sock_eq(node_map_s* self, void* ctx, node_s** device_p)
{
    zsock_t* eq = ctx;
    linq_netw_socket_s* socket = ((linq_netw_socket_s*)*device_p);
    if (eq == socket->sock) node_map_remove(self, node_serial(*device_p));
}

static void
remove_devices(zsock_t** s, device_map_s* devices)
{
    device_map_foreach(devices, foreach_device_remove_if_sock_eq, *s);
}

static void
remove_nodes(zsock_t** s, node_map_s* nodes)
{
    node_map_foreach(nodes, foreach_node_remove_if_sock_eq, *s);
}

E_LINQ_ERROR
linq_netw_close_router(linq_netw_s* l, linq_netw_socket handle)
{
    zsock_t** s = socket_map_resolve(l->zmtp.routers, handle);
    if (s) {
        remove_devices(s, l->devices);
        socket_map_remove_iter(l->zmtp.routers, handle);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

E_LINQ_ERROR
linq_netw_close_dealer(linq_netw_s* l, linq_netw_socket handle)
{
    zsock_t** s = socket_map_resolve(l->zmtp.dealers, handle);
    if (s) {
        remove_devices(s, l->devices);
        remove_nodes(s, l->nodes);
        socket_map_remove_iter(l->zmtp.dealers, handle);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

// poll network socket file handles
E_LINQ_ERROR
linq_netw_poll(linq_netw_s* l, int32_t ms)
{
    return zmtp_poll(&l->zmtp, ms);
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
