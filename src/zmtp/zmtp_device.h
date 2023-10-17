// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZMTP_DEVICE_H_
#define ZMTP_DEVICE_H_

// includes
#include "common/containers.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these
#include "zmtp.h"

#ifdef __cplusplus
extern "C"
{
#endif

    node_s* zmtp_device_create(
        zmq_socket_s* sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* type);
    void zmtp_device_destroy(node_s** d_p);
    const router_s* zmtp_device_router(node_s* d);
    void zmtp_device_update_router(node_s* d, const uint8_t* rid, uint32_t sz);
    void zmtp_device_send(
        node_s* d,
        E_REQUEST_METHOD method,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn,
        void* context);
    void zmtp_device_send_raw(
        node_s* d,
        const char* path,
        const char* json,
        linq_request_complete_fn fn,
        void* context);

    uint32_t zmtp_device_request_sent_at(node_s* d);
    uint32_t zmtp_device_request_retry_count(node_s* r);
    uint32_t zmtp_device_request_retry_at(node_s* d);
    void zmtp_device_request_retry_at_set(node_s* d, int);
    void zmtp_device_request_retry(node_s* d);
    void zmtp_device_request_resolve(node_s* d, int err, const char*);
    void zmtp_device_request_flush(node_s* d);
    void zmtp_device_request_flush_w_check(node_s* d);
    bool zmtp_device_request_pending(node_s* n, int32_t reqid);
    uint32_t zmtp_device_request_pending_count(node_s* d);
    void zmtp_device_poll(node_s* base, void* ctx);
    void zmtp_device_retry_timeout_set(int val);
    int zmtp_device_retry_timeout_get();
    void zmtp_device_max_retry_set(int val);
    int zmtp_device_max_retry_get();

#ifdef __cplusplus
}
#endif

#endif /* ZMTP_DEVICE_H_ */
