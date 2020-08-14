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
    uint32_t device_map_foreach_remove_if(
        device_map_s* hash,
        bool (*remove)(node_s*, void*),
        void* ctx);
    void device_map_foreach_poll(device_map_s*);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
