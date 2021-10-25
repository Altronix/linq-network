// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "containers.h"
#include "netw.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define devices_foreach(hash, iter)                                            \
    for (iter = devices_iter_start(hash); iter != devices_iter_end(hash);      \
         ++iter)

    typedef unsigned long long devices_iter;

    MAP_INIT_H(device, node_s);
    LINQ_EXPORT const char* device_type(node_s* d);
    LINQ_EXPORT const char* device_serial(node_s* d);
    LINQ_EXPORT E_TRANSPORT device_transport(node_s* d);
    LINQ_EXPORT uint32_t device_birth(node_s* d);
    LINQ_EXPORT uint32_t device_last_seen(node_s* d);
    LINQ_EXPORT uint32_t device_uptime(node_s* d);
    LINQ_EXPORT void device_heartbeat(node_s* d);
    LINQ_EXPORT bool device_legacy(node_s* d);
    LINQ_EXPORT bool device_legacy_set(node_s* d, bool);
    LINQ_EXPORT devices_iter devices_iter_start(device_map_s* map);
    LINQ_EXPORT devices_iter devices_iter_end(device_map_s* map);
    LINQ_EXPORT node_s* devices_iter_exist(device_map_s* map, devices_iter it);
    LINQ_EXPORT const char* devices_summary_alloc(device_map_s* map);
    LINQ_EXPORT void devices_summary_free(const char** mem_p);
    LINQ_EXPORT int device_map_print(device_map_s* d, char* b, uint32_t l);

    LINQ_EXPORT uint32_t devices_foreach_remove_if(
        device_map_s* hash,
        bool (*remove)(node_s*, void*),
        void* ctx);
    LINQ_EXPORT void devices_foreach_poll(device_map_s*);
    LINQ_EXPORT device_map_s* devices_create();
    LINQ_EXPORT void devices_destroy();
    LINQ_EXPORT node_s** devices_add(device_map_s*, const char*, node_s**);
    LINQ_EXPORT void devices_remove_iter(device_map_s* nodes, khiter_t k);
    LINQ_EXPORT void devices_remove(device_map_s* nodes, const char* serial);
    LINQ_EXPORT node_s** devices_get(device_map_s* hash, const char* serial);
    LINQ_EXPORT node_s** devices_resolve(device_map_s* hash, khiter_t k);
    LINQ_EXPORT khiter_t devices_key(device_map_s* hash, const char* serial);
    LINQ_EXPORT uint32_t devices_size(device_map_s* hash);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
