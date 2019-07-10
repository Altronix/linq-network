#include "device_map.h"
#include "device.h"

KHASH_MAP_INIT_STR(dev, device*);

typedef struct device_map
{
    kh_dev_t* map;
} device_map;

device_map*
device_map_create()
{
    device_map* d = linq_malloc(sizeof(device_map));
    if (d) { d->map = kh_init(dev); }
    return d;
}

void
device_map_destroy(device_map** d_p)
{
    device_map* map = *d_p;
    *d_p = NULL;
    for (khint_t k = kh_begin(map->map); k != kh_end(map->map); ++k) {
        if (kh_exist(map->map, k)) {
            // device* d = kh_val(map->map, k);
            // device_destroy(&d);
        }
    }
    kh_destroy(dev, map->map);
    linq_free(map);
}

void
device_map_insert(
    device_map* map,
    zsock_t** sock_p,
    zframe_t* router,
    zframe_t* serial,
    zframe_t* product)
{
    int ret;
    device* d = device_create(sock_p, router, serial, product);
    kh_put(dev, map->map, device_serial(d), &ret);
    ((void)ret);
}

uint32_t
device_map_size(device_map* map)
{
    return kh_size(map->map);
}
