// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netw.h"
#include "common/log.h"
#include "common/sys/sys.h"
#include "netw_internal.h"
#include "zmtp/callbacks.h"
#include "zmtp/node.h"
#include "zmtp/zmtp.h"
#include "zmtp/zmtp_device.h"

#define netw_info(...) log_info("NETW", __VA_ARGS__)
#define netw_warn(...) log_warn("NETW", __VA_ARGS__)
#define netw_debug(...) log_debug("NETW", __VA_ARGS__)
#define netw_trace(...) log_trace("NETW", __VA_ARGS__)
#define netw_error(...) log_error("NETW", __VA_ARGS__)
#define netw_fatal(...) log_fatal("NETW", __VA_ARGS__)

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
        sys_uuid(l->uuid);
        zmtp_init(&l->zmtp, &l->devices, &l->nodes, &zmtp_callbacks, l);
#ifdef BUILD_USBH
        usbh_init(&l->usb, &l->devices);
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
    netw_error("http support not enabled!");
}

// Listen for incoming device connections on "endpoint"
netw_socket
netw_listen(netw_s* l, const char* ep)
{
    int ep_len = strlen(ep);
    if (*ep == 'i' || *ep == 't') {
        netw_info("Listening... [%s]", ep);
        return zmtp_listen(&l->zmtp, ep);
    } else {
        netw_error("not supported");
        return -1;
    }
}

netw_socket
netw_connect(netw_s* l, const char* ep)
{
    netw_socket s;
    netw_info("Connecting... [%s]", ep);
    s = zmtp_connect(&l->zmtp, ep);
    netw_info("Connecting result [%d]", s);
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
    if (err < 0) netw_error("polling error %d", err);

#if BUILD_USBH
    err = usbh_poll(&l->usb, ms);
    if (err < 0) netw_error("polling error %d", err);
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

// Print a summary in json
LINQ_EXPORT const char*
netw_devices_summary_alloc(const netw_s* l)
{
    return devices_summary_alloc(l->devices);
}

// free json summary
LINQ_EXPORT void
netw_devices_summary_free(const char** alloc_p)
{
    devices_summary_free(alloc_p);
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
    netw_info("send [%.6s...] [%s] [%.*s]", serial, method, plen, path);
    node_s** node = devices_get(linq->devices, serial);
    if (!node) {
        netw_error("send [%.6s...] not found!", serial);
        m = "not found";
        snprintf(e, sizeof(e), "{\"error\":\"%s\"}", m);
        error = LINQ_ERROR_DEVICE_NOT_FOUND;
        fn(context, "", error, e);
    } else {
        netw_info("send [%.6s...] sent!", serial);
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

LINQ_EXPORT void
netw_retry_timeout_set(int val)
{
    zmtp_device_retry_timeout_set(val);
}

LINQ_EXPORT int
netw_retry_timeout_get()
{
    return zmtp_device_retry_timeout_get();
}

LINQ_EXPORT void
netw_max_retry_set(int val)
{
    zmtp_device_max_retry_set(val);
}

LINQ_EXPORT int
netw_max_retry_get()
{
    return zmtp_device_max_retry_get();
}

void
netw_shutdown(netw_s* netw)
{
    netw->shutdown = true;
    if (netw->callbacks && netw->callbacks->on_ctrlc) {
        netw->callbacks->on_ctrlc(netw->context);
    }
}

LINQ_EXPORT void
netw_log_fn_set(log_callback_fn fn, void* ctx)
{
    netw_info("Log output redirecting...");
    log_set_callback_fn(fn, ctx);
}

LINQ_EXPORT int
netw_scan(netw_s* linq)
{
#ifdef BUILD_USBH
    return usbh_scan(&linq->usb, 0x3333, 0x4444);
#else
    netw_error("usb support not compiled into library!");
    return -1;
#endif
}

LINQ_EXPORT const char*
netw_strerror(E_LINQ_ERROR e)
{
    static const char* error = "TODO";
    return error;
}
