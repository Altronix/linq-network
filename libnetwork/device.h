// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "containers.h"
#include "czmq.h"
#include "netw_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // typedef struct device_zmtp_s device_zmtp_s;
    MAP_INIT_H(device, node_s);

    // node_s* device_create(
    //     zsock_t* sock_p,
    //     const uint8_t* router,
    //     uint32_t router_sz,
    //     const char* serial,
    //     const char* type);
    // void device_destroy(node_s** d_p);
    // const char* device_serial(node_s* d);
    // const char* device_type(node_s* d);
    // const router_s* device_router(node_s* d);
    // bool device_no_hops(node_s* d);
    // bool device_hops(node_s* d);
    // void device_update_router(node_s* d, const uint8_t* rid, uint32_t sz);
    // uint32_t device_last_seen(node_s* d);
    // uint32_t device_uptime(node_s* d);
    // void device_heartbeat(node_s* d);
    // void device_send(
    //     node_s* d,
    //     E_REQUEST_METHOD method,
    //     const char* path,
    //     uint32_t plen,
    //     const char* json,
    //     uint32_t jlen,
    //     linq_request_complete_fn fn,
    //     void* context);
    // void device_send_raw(
    //     node_s* d,
    //     const char* path,
    //     const char* json,
    //     linq_request_complete_fn fn,
    //     void* context);

    // uint32_t device_request_sent_at(node_s* d);
    // uint32_t device_request_retry_count(node_s* r);
    // uint32_t device_request_retry_at(node_s* d);
    // void device_request_retry_at_set(node_s* d, uint32_t);
    // void device_request_resolve(node_s* d, int err, const char*);
    // void device_request_flush(node_s* d);
    // void device_request_flush_w_check(node_s* d);
    // void device_request_retry(node_s* d);
    // bool device_request_pending(node_s* n);
    // uint32_t device_request_pending_count(node_s* d);
    uint32_t device_map_foreach_remove_if_sock_eq(device_map_s*, zsock_t* z);
    void device_map_foreach_poll(device_map_s*);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
