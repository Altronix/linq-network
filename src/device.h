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
        router* router,
        const char* serial,
        const char* product);
    void device_destroy(device** d_p);
    const char* device_serial(device* d);
    const char* device_product(device* d);
    const router* device_router(device* d);
    void device_update_router(device* d, router* router);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
