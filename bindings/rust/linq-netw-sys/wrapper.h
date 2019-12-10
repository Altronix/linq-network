// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_H_
#define LINQ_H_

#include <stdbool.h>
#include <stdint.h>

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

    typedef struct device_s device_s;
    typedef struct linq_netw_s linq_netw_s;
    typedef uint32_t linq_netw_socket;

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

    typedef struct linq_netw_alert_s
    {
        const char* who;
        const char* what;
        const char* where;
        const char* when;
        const char* mesg;
        const char* email[5];
        char* data;
    } linq_netw_alert_s;

    typedef struct linq_netw_email_s
    {
        const char* to0;
        const char* to1;
        const char* to2;
        const char* to3;
        const char* to4;
        const char* from;
        const char* subject;
        const char* user;
        const char* password;
        const char* server;
        const char* port;
        const char* device;
        char* data;
    } linq_netw_email_s;

    typedef void (*linq_netw_request_complete_fn)(
        void*,
        E_LINQ_ERROR e,
        const char* json,
        device_s**);
    typedef void (
        *linq_netw_error_fn)(void*, E_LINQ_ERROR, const char*, const char*);
    typedef void (*linq_netw_heartbeat_fn)(void*, const char*, device_s**);
    typedef void (*linq_netw_alert_fn)(
        void*,
        linq_netw_alert_s*,
        linq_netw_email_s*,
        device_s**);
    typedef void (*linq_netw_ctrlc_fn)(void*);
    typedef void (
        *linq_netw_devices_foreach_fn)(void* ctx, const char*, const char*);
    typedef struct linq_netw_callbacks
    {
        linq_netw_error_fn err;
        linq_netw_heartbeat_fn hb;
        linq_netw_alert_fn alert;
        linq_netw_ctrlc_fn ctrlc;
    } linq_netw_callbacks;

    // Linq API
    linq_netw_s* linq_netw_create(const linq_netw_callbacks*, void*);
    void linq_netw_destroy(linq_netw_s**);
    void linq_netw_init(linq_netw_s*, const linq_netw_callbacks*, void*);
    void linq_netw_deinit(linq_netw_s*);
    void linq_netw_context_set(linq_netw_s* linq, void* ctx);
    linq_netw_socket linq_netw_listen(linq_netw_s*, const char* ep);
    linq_netw_socket linq_netw_connect(linq_netw_s* l, const char* ep);
    E_LINQ_ERROR linq_netw_close_router(linq_netw_s*, linq_netw_socket);
    E_LINQ_ERROR linq_netw_close_dealer(linq_netw_s*, linq_netw_socket);
    E_LINQ_ERROR linq_netw_poll(linq_netw_s* l, int32_t ms);
    device_s** linq_netw_device(const linq_netw_s*, const char*);
    uint32_t linq_netw_device_count(const linq_netw_s*);
    void linq_netw_devices_foreach(
        const linq_netw_s* l,
        linq_netw_devices_foreach_fn,
        void*);
    uint32_t linq_netw_nodes_count(const linq_netw_s* linq);
    E_LINQ_ERROR linq_netw_device_send_get(
        const linq_netw_s*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_netw_device_send_post(
        const linq_netw_s*,
        const char*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_netw_device_send_delete(
        const linq_netw_s*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);

    // Device API
    void device_send_delete(
        device_s*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    void device_send_get(
        device_s*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    void device_send_post(
        device_s*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    void device_send(
        device_s* d,
        const char* path,
        const char* json,
        linq_netw_request_complete_fn fn,
        void* context);
    const char* device_serial(device_s* d);

    // Sys API
    bool sys_running();
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
