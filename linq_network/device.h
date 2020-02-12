// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "containers.h"
#include "czmq.h"
#include "linq_network_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    MAP_INIT_H(device, device_s);

    device_s* device_create(
        zsock_t* sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* type);
    void device_destroy(device_s** d_p);
    const char* device_serial(device_s* d);
    const char* device_type(device_s* d);
    const router_s* device_router(device_s* d);
    bool device_no_hops(device_s* d);
    bool device_hops(device_s* d);
    zsock_t* device_socket(device_s* d);
    void device_update_router(device_s* d, const uint8_t* rid, uint32_t sz);
    uint32_t device_last_seen(device_s* d);
    uint32_t device_uptime(device_s* d);
    void device_heartbeat(device_s* d);
    void device_send_delete(
        device_s*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    void device_send_delete_mem(
        device_s*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    void device_send_get(
        device_s*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    void device_send_get_mem(
        device_s*,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    void device_send_post(
        device_s*,
        const char*,
        const char*,
        linq_network_request_complete_fn,
        void*);
    void device_send_post_mem(
        device_s*,
        const char*,
        uint32_t,
        const char*,
        uint32_t,
        linq_network_request_complete_fn,
        void*);
    void device_send(
        device_s* d,
        const char* path,
        const char* json,
        linq_network_request_complete_fn fn,
        void* context);
    uint32_t device_request_sent_at(device_s* d);
    uint32_t device_request_retry_count(device_s* r);
    uint32_t device_request_retry_at(device_s* d);
    void device_request_retry_at_set(device_s* d, uint32_t);
    void device_request_resolve(device_s* d, int err, const char*);
    void device_request_flush(device_s* d);
    void device_request_flush_w_check(device_s* d);
    void device_request_retry(device_s* d);
    bool device_request_pending(device_s* n);
    uint32_t device_request_pending_count(device_s* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
