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
        zframe_t* router,
        zframe_t* serial,
        zframe_t* product);
    void device_destroy(device** d_p);
    const char* device_serial(device* d);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
