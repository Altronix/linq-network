#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "czmq.h"
#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    device* device_create(
        zsock_t** sock_p,
        const uint8_t* router,
        uint32_t router_sz,
        const char* serial,
        const char* product);
    void device_destroy(device** d_p);
    const char* device_serial(device* d);
    const char* device_product(device* d);
    const router* device_router(device* d);
    void device_update_router(device* d, const uint8_t* rid, uint32_t sz);
    uint32_t device_last_seen(device* d);
    uint32_t device_uptime(device* d);
    void device_heartbeat(device* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
