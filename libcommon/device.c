// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "device.h"
#include "containers.h"
#include "log.h"

#define SCAN_FMT                                                               \
    "\"%s\":{"                                                                 \
    "\"idVendor\":%s,"                                                         \
    "\"idProduct\":%s,"                                                        \
    "\"manufacturer\":\"%s\","                                                 \
    "\"product\":\"%s\""                                                       \
    "}"

static void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    node->free(node_p);
}

MAP_INIT(device, node_s, node_destroy);

const char*
device_type(node_s* d)
{
    return d->type;
}

const char*
device_serial(node_s* d)
{
    return d->serial;
}

uint32_t
device_last_seen(node_s* d)
{
    return d->last_seen;
}

uint32_t
device_uptime(node_s* d)
{
    return d->last_seen - d->birth;
}

void
device_heartbeat(node_s* d)
{
    d->last_seen = sys_tick();
}

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

int
device_map_print(device_map_s* d, char* b, uint32_t l)
{
    uint32_t n = device_map_size(d), sz = l;
    l = 1;
    if (sz) *b = '{';
    node_s* device;
    device_iter iter;
    map_foreach(d, iter)
    {
        if (map_has_key(d, iter) && (device = map_val(d, iter))) {
            l += snprintf(
                &b[l],
                sz - l,
                SCAN_FMT,
                device->serial,
                "0x????",
                "0x????",
                "Altronix Corp.",
                device->type);
            if (--n) {
                if (l < sz) b[(l)++] = ',';
            }
        }
    }
    if (l < sz) b[(l)++] = '}';
    if (l < sz) b[(l)] = '\0';
    return l;
}