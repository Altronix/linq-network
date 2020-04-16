// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZMTP_H
#define ZMTP_H

#include "containers.h"
#include "device.h"
#include "linq_network_internal.h"
#include "node.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
    typedef void (
        *zmtp_cb_error_fn)(void*, E_LINQ_ERROR, const char*, const char*);
    typedef void (*zmtp_cb_new_fn)(void*, const char*);
    typedef void (*zmtp_cb_heartbeat_fn)(void*, const char*);
    typedef void (*zmtp_cb_ctrlc_fn)(void*);
    typedef void (*zmtp_cb_alert_fn)(
        void*,
        const char*,
        linq_network_alert_s*,
        linq_network_email_s*);
    typedef struct zmtp_callbacks_s
    {
        zmtp_cb_error_fn on_err;
        zmtp_cb_new_fn on_new;
        zmtp_cb_heartbeat_fn on_heartbeat;
        zmtp_cb_alert_fn on_alert;
        zmtp_cb_ctrlc_fn on_ctrlc;
    } zmtp_callbacks_s;
    */

    MAP_INIT_H(socket, zsock_t);
    typedef struct zmtp_s
    {
        void* context;
        socket_map_s* routers;
        socket_map_s* dealers;
        device_map_s** devices_p;
        node_map_s** nodes_p;
        bool shutdown;
        const linq_network_callbacks* callbacks;
    } zmtp_s;

    void zmtp_init(
        zmtp_s* zmtp,
        device_map_s** devices_p,
        node_map_s** nodes_p,
        const linq_network_callbacks* callbacks,
        void* ctx);
    void zmtp_deinit(zmtp_s* zmtp);
    linq_network_socket zmtp_listen(zmtp_s* zmtp, const char* ep);
    linq_network_socket zmtp_connect(zmtp_s* l, const char* ep);
    E_LINQ_ERROR zmtp_close_router(zmtp_s* zmtp, linq_network_socket sock);
    E_LINQ_ERROR zmtp_close_dealer(zmtp_s* zmtp, linq_network_socket sock);
    E_LINQ_ERROR
    zmtp_poll(zmtp_s* self, int32_t ms);
    E_LINQ_ERROR zmtp_device_send_get(
        const zmtp_s*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_get_mem(
        const zmtp_s*,
        const char*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_post(
        const zmtp_s*,
        const char*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_post_mem(
        const zmtp_s*,
        const char*,
        const char*,
        uint32_t,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_delete(
        const zmtp_s*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    E_LINQ_ERROR zmtp_device_send_delete_mem(
        const zmtp_s*,
        const char*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
#ifdef __cplusplus
}
#endif
#endif
