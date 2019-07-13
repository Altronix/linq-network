#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "czmq.h"
#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    device_s* device_create(
        zsock_t** sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* product);
    void device_destroy(device_s** d_p);
    const char* device_serial(device_s* d);
    const char* device_product(device_s* d);
    const router_s* device_router(device_s* d);
    void device_update_router(device_s* d, const uint8_t* rid, uint32_t sz);
    uint32_t device_last_seen(device_s* d);
    uint32_t device_uptime(device_s* d);
    void device_heartbeat(device_s* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
