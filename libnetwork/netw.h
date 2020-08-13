// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_H_
#define LINQ_H_

#include "sys.h"

#include "common.h"

// clang-format off
#if defined _WIN32
#  if defined LINQ_STATIC
#    define LINQ_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_EXPORT __declspec(dllexport)
#  else
#    define LINQ_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_EXPORT
#endif
// clang-format on

#define LINQ_ERROR_SOCKET 0xFFFFFFFF

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LINQ_NETW_MAX_RETRY
#define LINQ_NETW_MAX_RETRY 5
#endif

#ifndef LINQ_NETW_RETRY_TIMEOUT
#define LINQ_NETW_RETRY_TIMEOUT 500
#endif

#ifndef LINQ_NETW_MAX_RESPONSE_SIZE
#define LINQ_NETW_MAX_RESPONSE_SIZE 8096
#endif

    typedef struct database_s database_s;
    typedef struct netw_s netw_s;
    typedef uint32_t netw_socket;

    typedef struct atx_str
    {
        const char* p;
        uint32_t len;
    } atx_str;

    typedef struct netw_alert_s
    {
        atx_str who;
        atx_str what;
        atx_str where;
        atx_str when;
        atx_str mesg;
        atx_str name;
        atx_str product;
        atx_str email[5];
        char* data;
    } netw_alert_s;

    typedef struct netw_email_s
    {
        atx_str to0;
        atx_str to1;
        atx_str to2;
        atx_str to3;
        atx_str to4;
        atx_str from;
        atx_str subject;
        atx_str user;
        atx_str password;
        atx_str server;
        atx_str port;
        atx_str device;
        char* data;
    } netw_email_s;

    // REQUEST COMPLETE
    typedef void (*netw_request_complete_fn)(
        void*,
        const char* serial,
        E_LINQ_ERROR e,
        const char* json);
    // ERROR
    typedef void (*netw_error_fn)(
        void* context,
        E_LINQ_ERROR error,
        const char*,
        const char*);
    // NEW
    typedef void (*netw_new_fn)(void* context, const char* serial);
    // HEARTBEAT
    typedef void (*netw_heartbeat_fn)(void* context, const char* serial);
    // ALERT
    typedef void (*netw_alert_fn)(
        void* context,
        const char* serial,
        netw_alert_s*,
        netw_email_s*);
    // CTRLC
    typedef void (*netw_ctrlc_fn)(void* context);
    typedef void (
        *netw_devices_foreach_fn)(void* ctx, const char*, const char*);
    typedef struct netw_callbacks
    {
        netw_error_fn on_err;
        netw_new_fn on_new;
        netw_heartbeat_fn on_heartbeat;
        netw_alert_fn on_alert;
        netw_ctrlc_fn on_ctrlc;
    } netw_callbacks;

    // Linq API
    LINQ_EXPORT netw_s* netw_create(const netw_callbacks*, void*);
    LINQ_EXPORT void netw_destroy(netw_s**);
    LINQ_EXPORT database_s* netw_database(netw_s* l);
    LINQ_EXPORT void netw_init(netw_s*, const netw_callbacks*, void*);
    LINQ_EXPORT void netw_deinit(netw_s*);
    LINQ_EXPORT void netw_context_set(netw_s* linq, void* ctx);
    LINQ_EXPORT netw_socket netw_listen(netw_s*, const char* ep);
    LINQ_EXPORT netw_socket netw_connect(netw_s* l, const char* ep);
    LINQ_EXPORT E_LINQ_ERROR netw_close(netw_s*, netw_socket);
    LINQ_EXPORT E_LINQ_ERROR netw_poll(netw_s* l, int32_t ms);
    // TODO netw_device() is deprecated but it is currently used by some
    // tests...
    LINQ_EXPORT void** netw_device(const netw_s* l, const char* serial);
    LINQ_EXPORT bool netw_device_exists(const netw_s*, const char* sid);
    LINQ_EXPORT uint32_t netw_device_count(const netw_s*);
    LINQ_EXPORT void
    netw_devices_foreach(const netw_s* l, netw_devices_foreach_fn, void*);
    LINQ_EXPORT uint32_t netw_node_count(const netw_s* linq);
    E_LINQ_ERROR
    netw_send(
        const netw_s* linq,
        const char* sid,
        const char* meth,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        netw_request_complete_fn fn,
        void* ctx);

    // Sys API
    LINQ_EXPORT bool sys_running();
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
