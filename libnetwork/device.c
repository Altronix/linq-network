// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "device.h"
#include "containers.h"
#include "log.h"

static void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    node->free(node_p);
}

MAP_INIT(device, node_s, node_destroy);

uint32_t
device_map_foreach_remove_if(
    device_map_s* hash,
    bool (*remove)(node_s*, void*),
    void* ctx)
{
    uint32_t n = 0;
    map_iter iter;
    node_s* v;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter) && (v = map_val(hash, iter))) {
            if (remove(v, ctx)) {
                device_map_remove(hash, v->serial);
                n++;
            }
        }
    }
    return n;
}

void
device_map_foreach_poll(device_map_s* hash)
{
    node_s* base;
    map_iter iter;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter) && (base = map_val(hash, iter))) {
            base->poll(base, NULL);
        }
    }
}
