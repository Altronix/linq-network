#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "czmq.h"
#include "linq_internal.h"
#include "requests.h"

#ifdef __cplusplus
extern "C"
{
#endif

    node_s* node_create(
        zsock_t** sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* type);
    void node_destroy(node_s** d_p);
    const char* node_serial(node_s* d);
    const char* node_type(node_s* d);
    const router_s* node_router(node_s* d);
    void node_update_router(node_s* d, const uint8_t* rid, uint32_t sz);
    uint32_t node_last_seen(node_s* d);
    uint32_t node_uptime(node_s* d);
    void node_heartbeat(node_s* d);
    void node_send(node_s*, request_s**);
    void
    node_send_delete(node_s*, const char*, linq_request_complete_fn, void*);
    void node_send_frames(node_s* d, frames_s* forward);
    void node_send_get(node_s*, const char*, linq_request_complete_fn, void*);
    void node_send_post(
        node_s*,
        const char*,
        const char*,
        linq_request_complete_fn,
        void*);
    void node_resolve_request(node_s* d, int err, const char*);
    request_s* node_request_pending(node_s* n);
    uint32_t node_request_pending_count(node_s* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
