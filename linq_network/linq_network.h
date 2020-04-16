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

#ifndef LINQ_NETW_MAX_RESPONSE_SIZE
#define LINQ_NETW_MAX_RESPONSE_SIZE 8096
#endif

    typedef struct database_s database_s;
    typedef struct linq_network_s linq_network_s;
    typedef uint32_t linq_network_socket;

    typedef struct atx_str
    {
        const char* p;
        uint32_t len;
    } atx_str;

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

    typedef struct linq_network_alert_s
    {
        atx_str who;
        atx_str what;
        atx_str where;
        atx_str when;
        atx_str mesg;
        atx_str email[5];
        char* data;
    } linq_network_alert_s;

    typedef struct linq_network_email_s
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
    } linq_network_email_s;

    // REQUEST COMPLETE
    typedef void (*linq_network_request_complete_fn)(
        void*,
        const char* serial,
        E_LINQ_ERROR e,
        const char* json);
    // ERROR
    typedef void (*linq_network_error_fn)(
        void* context,
        E_LINQ_ERROR error,
        const char*,
        const char*);
    // NEW
    typedef void (*linq_network_new_fn)(void* context, const char* serial);
    // HEARTBEAT
    typedef void (
        *linq_network_heartbeat_fn)(void* context, const char* serial);
    // ALERT
    typedef void (*linq_network_alert_fn)(
        void* context,
        const char* serial,
        linq_network_alert_s*,
        linq_network_email_s*);
    // CTRLC
    typedef void (*linq_network_ctrlc_fn)(void* context);
    typedef void (
        *linq_network_devices_foreach_fn)(void* ctx, const char*, const char*);
    typedef struct linq_network_callbacks
    {
        linq_network_error_fn on_err;
        linq_network_new_fn on_new;
        linq_network_heartbeat_fn on_heartbeat;
        linq_network_alert_fn on_alert;
        linq_network_ctrlc_fn on_ctrlc;
    } linq_network_callbacks;

    // Linq API
    linq_network_s* linq_network_create(const linq_network_callbacks*, void*);
    void linq_network_destroy(linq_network_s**);
    database_s* linq_network_database(linq_network_s* l);
    void
    linq_network_init(linq_network_s*, const linq_network_callbacks*, void*);
    void linq_network_deinit(linq_network_s*);
    void linq_network_context_set(linq_network_s* linq, void* ctx);
    linq_network_socket linq_network_listen(linq_network_s*, const char* ep);
    linq_network_socket linq_network_connect(linq_network_s* l, const char* ep);
    E_LINQ_ERROR linq_network_close(linq_network_s*, linq_network_socket);
    E_LINQ_ERROR linq_network_poll(linq_network_s* l, int32_t ms);
    // TODO linq_network_device() is deprecated but it is currently used by some
    // tests...
    void** linq_network_device(const linq_network_s* l, const char* serial);
    bool linq_network_device_exists(const linq_network_s*, const char* sid);
    uint32_t linq_network_device_count(const linq_network_s*);
    void linq_network_devices_foreach(
        const linq_network_s* l,
        linq_network_devices_foreach_fn,
        void*);
    uint32_t linq_network_node_count(const linq_network_s* linq);
    E_LINQ_ERROR linq_network_send_get(
        const linq_network_s*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_network_send_get_mem(
        const linq_network_s*,
        const char*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_network_send_post(
        const linq_network_s*,
        const char*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_network_send_post_mem(
        const linq_network_s*,
        const char*,
        const char*,
        uint32_t,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_network_send_delete(
        const linq_network_s*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR
    linq_network_send_delete_mem(
        const linq_network_s*,
        const char*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);

    // Sys API
    bool sys_running();
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
