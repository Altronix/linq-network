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

    typedef struct device_s device_s;
    typedef struct atx_net_s atx_net_s;
    typedef uint32_t atx_net_socket;

    typedef struct linq_str
    {
        const char* p;
        uint32_t len;
    } linq_str;

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

    typedef struct atx_net_alert_s
    {
        linq_str who;
        linq_str what;
        linq_str where;
        linq_str when;
        linq_str mesg;
        linq_str email[5];
        char* data;
    } atx_net_alert_s;

    typedef struct atx_net_email_s
    {
        linq_str to0;
        linq_str to1;
        linq_str to2;
        linq_str to3;
        linq_str to4;
        linq_str from;
        linq_str subject;
        linq_str user;
        linq_str password;
        linq_str server;
        linq_str port;
        linq_str device;
        char* data;
    } atx_net_email_s;

    typedef void (*atx_net_request_complete_fn)(
        void*,
        E_LINQ_ERROR e,
        const char* json,
        device_s**);
    typedef void (
        *atx_net_error_fn)(void*, E_LINQ_ERROR, const char*, const char*);
    typedef void (*atx_net_heartbeat_fn)(void*, const char*, device_s**);
    typedef void (*atx_net_alert_fn)(
        void*,
        atx_net_alert_s*,
        atx_net_email_s*,
        device_s**);
    typedef void (*atx_net_ctrlc_fn)(void*);
    typedef void (
        *atx_net_devices_foreach_fn)(void* ctx, const char*, const char*);
    typedef struct atx_net_callbacks
    {
        atx_net_error_fn err;
        atx_net_heartbeat_fn hb;
        atx_net_alert_fn alert;
        atx_net_ctrlc_fn ctrlc;
    } atx_net_callbacks;

    // Linq API
    atx_net_s* atx_net_create(const atx_net_callbacks*, void*);
    void atx_net_destroy(atx_net_s**);
    void atx_net_init(atx_net_s*, const atx_net_callbacks*, void*);
    void atx_net_deinit(atx_net_s*);
    void atx_net_context_set(atx_net_s* linq, void* ctx);
    atx_net_socket atx_net_listen(atx_net_s*, const char* ep);
    atx_net_socket atx_net_connect(atx_net_s* l, const char* ep);
    E_LINQ_ERROR atx_net_close_router(atx_net_s*, atx_net_socket);
    E_LINQ_ERROR atx_net_close_dealer(atx_net_s*, atx_net_socket);
    E_LINQ_ERROR atx_net_close_http(atx_net_s*, atx_net_socket);
    E_LINQ_ERROR atx_net_poll(atx_net_s* l, int32_t ms);
    device_s** atx_net_device(const atx_net_s*, const char*);
    uint32_t atx_net_device_count(const atx_net_s*);
    void atx_net_devices_foreach(
        const atx_net_s* l,
        atx_net_devices_foreach_fn,
        void*);
    uint32_t atx_net_node_count(const atx_net_s* linq);
    E_LINQ_ERROR atx_net_send_get(
        const atx_net_s*,
        const char*,
        const char*,
        atx_net_request_complete_fn,
        void*);
    E_LINQ_ERROR atx_net_send_post(
        const atx_net_s*,
        const char*,
        const char*,
        const char*,
        atx_net_request_complete_fn,
        void*);
    E_LINQ_ERROR atx_net_send_delete(
        const atx_net_s*,
        const char*,
        const char*,
        atx_net_request_complete_fn,
        void*);

    // Device API
    void device_send_delete(
        device_s*,
        const char*,
        atx_net_request_complete_fn,
        void*);
    void
    device_send_get(device_s*, const char*, atx_net_request_complete_fn, void*);
    void device_send_post(
        device_s*,
        const char*,
        const char*,
        atx_net_request_complete_fn,
        void*);
    void device_send(
        device_s* d,
        const char* path,
        const char* json,
        atx_net_request_complete_fn fn,
        void* context);
    const char* device_serial(device_s* d);

    // Sys API
    bool sys_running();
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
