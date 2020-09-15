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

LINQ_EXPORT const char*
netw_version()
{
#ifdef LINQ_NETWORK_VERSION
    static const char* version = LINQ_NETWORK_VERSION;
#else
    static const char* version = "undefined";
#endif
    return version;
}

// Create main context for the caller
netw_s*
netw_create(const netw_callbacks* cb, void* context)
{
    netw_s* l = linq_network_malloc(sizeof(netw_s));
    if (l) {
#if defined _WIN32
        zmtp_callbacks_init();
#endif
        memset(l, 0, sizeof(netw_s));
        l->devices = devices_create();
        l->nodes = node_map_create();
        l->callbacks = cb;
        l->context = context ? context : l;
        zmtp_init(&l->zmtp, &l->devices, &l->nodes, &zmtp_callbacks, l);
#ifdef BUILD_USBH
        usbh_init(&l->usb, &l->devices);
#endif

#ifdef BUILD_LINQD
#define ADD_ROUTE(http, path, fn, ctx) http_use(http, path, fn, ctx)
        database_init(&l->database);
        http_init(&l->http, &l->database);
        ADD_ROUTE(&l->http, "/api/v1/devices", route_devices, l);
        ADD_ROUTE(&l->http, "/api/v1/alerts", route_alerts, l);
        ADD_ROUTE(&l->http, "/api/v1/exe/scan", route_scan, l);
        ADD_ROUTE(&l->http, "/api/v1/exe/kill", route_quit, l);
        ADD_ROUTE(&l->http, "/api/v1/proxy/...", route_proxy, l);
        ADD_ROUTE(&l->http, "/api/v1/connect", route_connect, l);
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
    devices_destroy(&l->devices);
    node_map_destroy(&l->nodes);

#if BUILD_USBH
    usbh_free(&l->usb);
#endif

#if BUILD_LINQD
    http_deinit(&l->http);
    database_deinit(&l->database);
#endif
    zmtp_deinit(&l->zmtp);
    linq_network_free(l);
}

void
netw_context_set(netw_s* linq, void* ctx)
{
    linq->context = ctx;
}

void
netw_root(netw_s* linq, const char* root)
{
#ifdef BUILD_LINQD
    http_root(&linq->http, root);
#else
    log_error("(HTTP) http support not enabled!");
#endif
}

// Listen for incoming device connections on "endpoint"
netw_socket
netw_listen(netw_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    if (*ep == 'i' || *ep == 't') {
        log_info("(ZMTP) Listening... [%s]", ep);
        return zmtp_listen(&l->zmtp, ep);
    } else {
#ifdef BUILD_LINQD
        if (ep_len > 9 && !memcmp(ep, "http://*:", 9)) {
            http_listen(&l->http, ep + 9);
            return LINQ_ERROR_OK;
        } else {
            http_listen(&l->http, ep);
            return LINQ_ERROR_OK;
        }
#else
        log_error("(HTTP) not supported");
        return -1;
#endif
    }
}

netw_socket
netw_connect(netw_s* l, const char* ep)
{
    netw_socket s;
    log_info("(ZMTP) Connecting... [%s]", ep);
    s = zmtp_connect(&l->zmtp, ep);
    log_info("(ZMTP) Connecting result [%d]", s);
    return s;
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
    if (err < 0) log_error("(ZMTP) polling error %d", err);

#if BUILD_USBH
    err = usbh_poll(&l->usb, ms);
    if (err < 0) log_error("(USB) polling error %d", err);
#endif

#if BUILD_LINQD
    err = http_poll(&l->http, ms);
    if (err < 0) log_error("(HTTP) polling error %d", err);
#endif

    // Loop through devices
    devices_foreach_poll(l->devices);

    return err;
}

// get a device from the device map
node_s**
netw_device(const netw_s* l, const char* serial)
{
    return devices_get(l->devices, serial);
}

// Check if the serial number is known in our hash table
bool
netw_device_exists(const netw_s* linq, const char* sid)
{
    return devices_get(linq->devices, sid) ? true : false;
}

// return how many devices are connected to linq
uint32_t
netw_device_count(const netw_s* l)
{
    return devices_size(l->devices);
}

int
netw_device_remove(const netw_s* l, const char* sid)
{
    if (netw_device_exists(l, sid)) {
        devices_remove(l->devices, sid);
        return 0;
    } else {
        return -1;
    }
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

// TODO move to common
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
    const char* m;
    node_s** node = devices_get(linq->devices, serial);
    if (!node) {
        m = "not found";
        snprintf(e, sizeof(e), "{\"error\":\"%s\"}", m);
        error = LINQ_ERROR_DEVICE_NOT_FOUND;
        fn(context, "", error, e);
    } else {
        E_REQUEST_METHOD m = method_from_str(method);
        (*node)->send(*node, m, path, plen, json, jlen, fn, context);
    }
    return error;
}

bool
netw_running(netw_s* netw)
{
    return sys_running() && !netw->shutdown;
}

void
netw_shutdown(netw_s* netw)
{
    netw->shutdown = true;
    if (netw->callbacks && netw->callbacks->on_ctrlc) {
        netw->callbacks->on_ctrlc(netw->context);
    }
}

LINQ_EXPORT int
netw_scan(netw_s* linq)
{
#ifdef BUILD_USBH
    return usbh_scan(&linq->usb, 0x3333, 0x4444);
#else
    log_error("(NETW) usb support not compiled into library!");
    return -1;
#endif
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
