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

    typedef struct devices_s devices_s;

    devices_s* devices_create();
    void devices_destroy(devices_s**);
    device_s** devices_insert(
        devices_s*,
        zsock_t** sock_p,
        uint8_t* r,
        uint32_t sz,
        const char* serial,
        const char* product);
    device_s** devices_get(devices_s* m, const char* serial);
    uint32_t devices_remove(devices_s* dmap, const char* serial);
    uint32_t devices_size(devices_s* map);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MAP_H_ */
