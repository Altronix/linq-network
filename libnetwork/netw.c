// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netw.h"
#include "log.h"
#include "node.h"
#include "sys.h"
#include "zmtp.h"
#ifdef BUILD_LINQD
#include "database.h"
#include "http.h"
#include "routes/routes.h"
#endif
#include "callbacks.h"
#include "netw_internal.h"

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
#ifdef BUILD_LINQD
#define ADD_ROUTE(http, path, fn, ctx) http_use(http, path, fn, ctx)
        database_init(&l->database);
        http_init(&l->http, &l->database);
        ADD_ROUTE(&l->http, "/api/v1/devices", devices, l);
        ADD_ROUTE(&l->http, "/api/v1/alerts", alerts, l);
        ADD_ROUTE(&l->http, "/api/v1/proxy/...", proxy, l);
#undef ADD_ROUTE
#endif
    }
    return l;
}

// Free main context after use
void
netw_destroy(netw_s** netw_p)
{
    netw_s* l = *netw_p;
    *netw_p = NULL;
#if BUILD_LINQD
    http_deinit(&l->http);
    database_deinit(&l->database);
#endif
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

netw_socket
netw_listen_http(netw_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    log_info("(ZMTP) Listening... [%s]", ep);
#ifdef BUILD_LINQD
    http_listen(&l->http, ep);
    return LINQ_ERROR_OK; // TODO return socket handle
#endif
    return LINQ_ERROR_PROTOCOL;
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

#if BUILD_LINQD
    err = http_poll(&l->http, ms);
    if (err) log_error("(HTTP) polling error %d", err);
#endif

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

bool
netw_running()
{
    return sys_running();
}

#ifdef BUILD_LINQD
LINQ_EXPORT database_s*
netw_database(netw_s* l)
{
    return &l->database;
}

LINQ_EXPORT void
netw_use(netw_s* netw, const char* path, http_route_cb cb, void* context)
{
    http_use(&netw->http, path, cb, context);
}
#endif
