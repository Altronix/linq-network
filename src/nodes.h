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

    typedef struct nodes_s nodes_s;
    typedef void (*nodes_foreach_fn)(void*, node_s**);

    nodes_s* nodes_create();
    void nodes_destroy(nodes_s**);
    node_s** nodes_add(nodes_s* nodes, node_s** node);
    node_s** nodes_get(nodes_s* m, const char* serial);
    uint32_t nodes_remove(nodes_s* dmap, const char* serial);
    uint32_t nodes_size(nodes_s* map);
    void nodes_foreach(nodes_s* n, nodes_foreach_fn fn, void* ctx);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MAP_H_ */
