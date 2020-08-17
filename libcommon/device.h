// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVICE_H_
#define DEVICE_H_

// includes
#include "common.h"
#include "containers.h"

#ifdef __cplusplus
extern "C"
{
#endif

    MAP_INIT_H(device, node_s);
    const char* device_type(node_s* d);
    const char* device_serial(node_s* d);
    uint32_t device_last_seen(node_s* d);
    uint32_t device_uptime(node_s* d);
    void device_heartbeat(node_s* d);
    int device_map_print(device_map_s* d, char* b, uint32_t l);
    uint32_t device_map_foreach_remove_if(
        device_map_s* hash,
        bool (*remove)(node_s*, void*),
        void* ctx);
    void device_map_foreach_poll(device_map_s*);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
