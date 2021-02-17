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

#define SUMMARY_FMT                                                            \
    "\"%s\":{"                                                                 \
    "\"serial\":\"%s\","                                                       \
    "\"type\":\"%s\","                                                         \
    "\"birth\":%d,"                                                            \
    "\"uptime\":%d,"                                                           \
    "\"lastSeen\":%d,"                                                         \
    "\"transport\":%d"                                                         \
    "}"

static void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    node->free(node_p);
}

MAP_INIT(device, node_s, node_destroy);

LINQ_EXPORT const char*
device_type(node_s* d)
{
    return d->type;
}

LINQ_EXPORT const char*
device_serial(node_s* d)
{
    return d->serial;
}

LINQ_EXPORT E_TRANSPORT
device_transport(node_s* d)
{
    return d->transport;
}

LINQ_EXPORT uint32_t
device_birth(node_s* d)
{
    return d->birth;
}

LINQ_EXPORT uint32_t
device_last_seen(node_s* d)
{
    return d->last_seen;
}

LINQ_EXPORT uint32_t
device_uptime(node_s* d)
{
    return d->last_seen - d->birth;
}

LINQ_EXPORT void
device_heartbeat(node_s* d)
{
    d->last_seen = sys_tick();
}

LINQ_EXPORT devices_iter
devices_iter_start(device_map_s* map)
{
    return kh_begin(map);
}

LINQ_EXPORT devices_iter
devices_iter_end(device_map_s* map)
{
    return kh_end(map);
}

LINQ_EXPORT node_s*
devices_iter_exist(device_map_s* map, devices_iter it)
{
    return map_has_key(map, it) ? map_val(map, it) : NULL;
}

LINQ_EXPORT const char*
devices_alloc_summary(device_map_s* map)
{
    char* alloc = NULL;
    uint32_t n = devices_size(map), spot = 0, l = (n + 1) * 512;
    alloc = linq_network_malloc(l);
    devices_iter i;
    node_s* node;
    if (alloc) {
        alloc[spot++] = '{';
        devices_foreach(map, i)
        {
            if ((node = devices_iter_exist(map, i))) {
                spot += snprintf(
                    &alloc[spot],
                    l - spot,
                    SUMMARY_FMT,
                    device_serial(node),
                    device_serial(node),
                    device_type(node),
                    device_birth(node),
                    device_uptime(node),
                    device_last_seen(node),
                    device_transport(node));
                if (--n && spot < l) alloc[(spot++)] = ',';
            }
        }
        if (spot < l) alloc[(spot)++] = '}';
        if (spot < l) alloc[(spot)++] = '\0';
    }
    return alloc;
}

LINQ_EXPORT uint32_t
devices_foreach_remove_if(
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

LINQ_EXPORT void
devices_foreach_poll(device_map_s* hash)
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

LINQ_EXPORT int
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

LINQ_EXPORT device_map_s*
devices_create()
{
    return device_map_create();
}

LINQ_EXPORT void
devices_destroy(device_map_s** map_p)
{
    device_map_destroy(map_p);
}

LINQ_EXPORT node_s**
devices_add(device_map_s* hash, const char* key, node_s** n)
{
    return device_map_add(hash, key, n);
}

LINQ_EXPORT void
devices_remove_iter(device_map_s* nodes, khiter_t k)
{
    device_map_remove_iter(nodes, k);
}

LINQ_EXPORT void
devices_remove(device_map_s* nodes, const char* serial)
{
    device_map_remove(nodes, serial);
}

LINQ_EXPORT node_s**
devices_get(device_map_s* hash, const char* serial)
{
    return device_map_get(hash, serial);
}

LINQ_EXPORT node_s**
devices_resolve(device_map_s* hash, khiter_t k)
{
    return device_map_resolve(hash, k);
}

LINQ_EXPORT khiter_t
devices_key(device_map_s* hash, const char* serial)
{
    return device_map_key(hash, serial);
}

LINQ_EXPORT uint32_t
devices_size(device_map_s* hash)
{
    return device_map_size(hash);
}
