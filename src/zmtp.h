#ifndef ZMTP_H
#define ZMTP_H

#include "containers.h"
#include "device.h"
#include "linq_netw_internal.h"
#include "node.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (
        *zmtp_cb_error_fn)(void*, E_LINQ_ERROR, const char*, const char*);
    typedef void (*zmtp_cb_heartbeat_fn)(void*, const char*, device_s**);
    typedef void (*zmtp_cb_ctrlc_fn)(void*);
    typedef void (*zmtp_cb_alert_fn)(
        void*,
        linq_netw_alert_s*,
        linq_netw_email_s*,
        device_s**);
    typedef struct zmtp_callbacks_s
    {
        zmtp_cb_error_fn err;
        zmtp_cb_heartbeat_fn hb;
        zmtp_cb_alert_fn alert;
        zmtp_cb_ctrlc_fn ctrlc;
    } zmtp_callbacks_s;

    MAP_INIT_H(socket, zsock_t);
    typedef struct zmtp_s
    {
        void* context;
        socket_map_s* routers;
        socket_map_s* dealers;
        device_map_s** devices_p;
        node_map_s** nodes_p;
        bool shutdown;
        const zmtp_callbacks_s* callbacks;
    } zmtp_s;

    void zmtp_init(
        zmtp_s* zmtp,
        device_map_s** devices_p,
        node_map_s** nodes_p,
        const zmtp_callbacks_s* callbacks,
        void* ctx);
    void zmtp_deinit(zmtp_s* zmtp);
    linq_netw_socket zmtp_listen(zmtp_s* zmtp, const char* ep);
    linq_netw_socket zmtp_connect(zmtp_s* l, const char* ep);
    E_LINQ_ERROR zmtp_close_router(zmtp_s* zmtp, linq_netw_socket sock);
    E_LINQ_ERROR zmtp_close_dealer(zmtp_s* zmtp, linq_netw_socket sock);
    E_LINQ_ERROR
    zmtp_poll(zmtp_s* self, int32_t ms);
    E_LINQ_ERROR zmtp_device_send_get(
        const zmtp_s*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_post(
        const zmtp_s*,
        const char*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_delete(
        const zmtp_s*,
        const char*,
        const char*,
        linq_netw_request_complete_fn,
        void*);
#ifdef __cplusplus
}
#endif
#endif
