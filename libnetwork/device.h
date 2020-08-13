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

    typedef struct device_zmtp_s device_zmtp_s;
    MAP_INIT_H(device, device_zmtp_s);

    device_zmtp_s* device_create(
        zsock_t* sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* type);
    void device_destroy(device_zmtp_s** d_p);
    const char* device_serial(device_zmtp_s* d);
    const char* device_type(device_zmtp_s* d);
    const router_s* device_router(device_zmtp_s* d);
    bool device_no_hops(device_zmtp_s* d);
    bool device_hops(device_zmtp_s* d);
    zsock_t* device_socket(device_zmtp_s* d);
    void
    device_update_router(device_zmtp_s* d, const uint8_t* rid, uint32_t sz);
    uint32_t device_last_seen(device_zmtp_s* d);
    uint32_t device_uptime(device_zmtp_s* d);
    void device_heartbeat(device_zmtp_s* d);
    E_REQUEST_METHOD device_method_from_str(const char*);
    void device_send(
        device_zmtp_s* d,
        E_REQUEST_METHOD method,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        netw_request_complete_fn fn,
        void* context);
    void device_send_raw(
        device_zmtp_s* d,
        const char* path,
        const char* json,
        netw_request_complete_fn fn,
        void* context);

    uint32_t device_request_sent_at(device_zmtp_s* d);
    uint32_t device_request_retry_count(device_zmtp_s* r);
    uint32_t device_request_retry_at(device_zmtp_s* d);
    void device_request_retry_at_set(device_zmtp_s* d, uint32_t);
    void device_request_resolve(device_zmtp_s* d, int err, const char*);
    void device_request_flush(device_zmtp_s* d);
    void device_request_flush_w_check(device_zmtp_s* d);
    void device_request_retry(device_zmtp_s* d);
    bool device_request_pending(device_zmtp_s* n);
    uint32_t device_request_pending_count(device_zmtp_s* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
