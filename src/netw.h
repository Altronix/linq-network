// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_H_
#define LINQ_H_

// clang-format off
#if defined _WIN32
#  include <assert.h>
#  include <stdarg.h>
#  include <stdbool.h>
#  include <stdint.h>
#  include <stdio.h>
#  include <string.h>
#  if defined LINQ_STATIC
#    define LINQ_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_EXPORT __declspec(dllexport)
#  else
#    define LINQ_EXPORT __declspec(dllimport)
#  endif
#else
#  include <assert.h>
#  include <stdarg.h>
#  include <stdbool.h>
#  include <stddef.h>
#  include <stdint.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#  define LINQ_EXPORT
#endif
// clang-format on

#define linq_network_malloc_fn malloc
#define linq_network_free_fn free
#define linq_network_assert_fn assert
#define linq_network_malloc(x) linq_network_malloc_fn(x)
#define linq_network_free(x) linq_network_free_fn(x)
#define linq_network_assert(x) linq_network_assert_fn(x)

#ifndef LINQ_SID_LEN
#define LINQ_SID_LEN 64
#endif

#ifndef LINQ_TID_LEN
#define LINQ_TID_LEN 64
#endif

#ifndef LOG_MESSAGE_LEN
#define LOG_MESSAGE_LEN 128
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        LINQ_TRACE,
        LINQ_DEBUG,
        LINQ_INFO,
        LINQ_WARN,
        LINQ_ERROR,
        LINQ_FATAL,
        LINQ_NONE
    } E_LOG_LEVEL;

    typedef struct log_callback_s
    {
        void* context;
        uint32_t line;
        uint32_t tick;
        const char* file;
        const char* level;
        const char* category;
        char message[LOG_MESSAGE_LEN];
    } log_callback_s;
    typedef void (*log_callback_fn)(log_callback_s*);

    typedef enum E_TRANSPORT
    {
        TRANSPORT_USB,
        TRANSPORT_ZMTP,
    } E_TRANSPORT;

    typedef enum E_REQUEST_METHOD
    {
        REQUEST_METHOD_RAW = 0,
        REQUEST_METHOD_GET = 1,
        REQUEST_METHOD_POST = 2,
        REQUEST_METHOD_DELETE = 3
    } E_REQUEST_METHOD;

    typedef enum
    {
        LINQ_ERROR_OK = 0,
        LINQ_ERROR_OOM = -1,
        LINQ_ERROR_BAD_ARGS = -2,
        LINQ_ERROR_PROTOCOL = -3,
        LINQ_ERROR_IO = -4,
        LINQ_ERROR_DEVICE_NOT_FOUND = -5,
        LINQ_ERROR_TIMEOUT = -6,
        LINQ_ERROR_SHUTTING_DOWN = -7,
        LINQ_ERROR_400 = 400,
        LINQ_ERROR_403 = 403,
        LINQ_ERROR_404 = 404,
        LINQ_ERROR_500 = 500,
        LINQ_ERROR_504 = 504,
    } E_LINQ_ERROR;

    // TODO need to have socket key as int32 instead of uint32
#define LINQ_ERROR_SOCKET 0xFFFFFFFF

    typedef void (*linq_request_complete_fn)(
        void*,
        const char* serial,
        E_LINQ_ERROR e,
        const char* json);

    typedef struct request_s
    {
        linq_request_complete_fn callback;
        uint32_t sent_at;
        uint32_t retry_at;
        uint32_t retry_count;
        void* ctx;
    } request_s;

    typedef struct node_s
    {
        request_s* pending;
        char serial[LINQ_SID_LEN];
        char type[LINQ_TID_LEN];
        uint32_t birth;
        uint32_t uptime;
        uint32_t last_seen;
        E_TRANSPORT transport;
        void (*poll)(struct node_s*, void*);
        void (*free)(struct node_s**);
        void (*send)(
            struct node_s*,
            E_REQUEST_METHOD method,
            const char* path,
            uint32_t plen,
            const char* json,
            uint32_t jlen,
            linq_request_complete_fn,
            void*);
    } node_s;

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
    typedef void (
        *netw_alert_fn)(void*, const char*, netw_alert_s*, netw_email_s*);
    // CTRLC
    typedef void (*netw_ctrlc_fn)(void*);
    typedef void (*netw_devices_foreach_fn)(void*, const char*, const char*);
    typedef struct netw_callbacks
    {
        netw_error_fn on_err;
        netw_new_fn on_new;
        netw_heartbeat_fn on_heartbeat;
        netw_alert_fn on_alert;
        netw_ctrlc_fn on_ctrlc;
    } netw_callbacks;

    // Linq API
    LINQ_EXPORT const char* netw_version();
    LINQ_EXPORT netw_s* netw_create(const netw_callbacks*, void*);
    LINQ_EXPORT void netw_destroy(netw_s**);
    LINQ_EXPORT void netw_init(netw_s*, const netw_callbacks*, void*);
    LINQ_EXPORT void netw_deinit(netw_s*);
    LINQ_EXPORT void netw_context_set(netw_s* linq, void* ctx);
    LINQ_EXPORT void netw_root(netw_s* linq, const char* root);
    LINQ_EXPORT netw_socket netw_listen(netw_s*, const char* ep);
    LINQ_EXPORT netw_socket netw_connect(netw_s* l, const char* ep);
    LINQ_EXPORT E_LINQ_ERROR netw_close(netw_s*, netw_socket);
    LINQ_EXPORT E_LINQ_ERROR netw_poll(netw_s* l, int32_t ms);
    LINQ_EXPORT node_s** netw_device(const netw_s* l, const char* serial);
    LINQ_EXPORT const char* netw_devices_summary_alloc(const netw_s* l);
    LINQ_EXPORT void netw_devices_summary_free(const char**);
    LINQ_EXPORT bool netw_device_exists(const netw_s*, const char* sid);
    LINQ_EXPORT uint32_t netw_device_count(const netw_s*);
    LINQ_EXPORT int netw_device_remove(const netw_s* l, const char* sid);
    LINQ_EXPORT void
    netw_devices_foreach(const netw_s* l, netw_devices_foreach_fn, void*);
    LINQ_EXPORT uint32_t netw_node_count(const netw_s* linq);
    LINQ_EXPORT E_LINQ_ERROR netw_send(
        const netw_s* linq,
        const char* sid,
        const char* meth,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn,
        void* ctx);
    LINQ_EXPORT bool netw_running(netw_s* netw);

    LINQ_EXPORT int netw_scan(netw_s*);
    LINQ_EXPORT void netw_retry_timeout_set(int);
    LINQ_EXPORT int netw_retry_timeout_get();
    LINQ_EXPORT void netw_max_retry_set(int);
    LINQ_EXPORT int netw_max_retry_get();
    LINQ_EXPORT void netw_shutdown(netw_s* netw);
    LINQ_EXPORT void netw_log_fn_set(log_callback_fn fn, void* ctx);

#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
