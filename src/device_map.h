#ifndef DEVICE_MAP_H_
#define DEVICE_MAP_H_

// includes
#include "czmq.h"
#include "klib/khash.h"

#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device_map device_map;

    device_map* device_map_create();
    void device_map_destroy(device_map**);
    device** device_map_insert(
        device_map*,
        zsock_t** sock_p,
        router* r,
        const char* serial,
        const char* product);
    device** device_map_get(device_map* m, const char* serial);
    uint32_t device_map_remove(device_map* dmap, const char* serial);
    uint32_t device_map_size(device_map* map);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MAP_H_ */
