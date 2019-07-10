#ifndef DEVICE_MAP_H_
#define DEVICE_MAP_H_

// includes

#ifdef __cplusplus
extern "C"
{
#endif

#include "czmq.h"
#include "klib/khash.h"
#include "linq_internal.h"

    typedef struct device_map device_map;

    device_map* device_map_create();
    void device_map_destroy(device_map**);
    void device_map_add(
        zsock_t** sock_p,
        zframe_t* router,
        zframe_t* serial,
        zframe_t* product);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MAP_H_ */
