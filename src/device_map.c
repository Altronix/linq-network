#include "device_map.h"
#include "device.h"

KHASH_MAP_INIT_STR(dev, device*);

typedef struct device_map
{
    kh_dev_t* h;
} device_map;

device_map*
device_map_create()
{
    device_map* d = linq_malloc(sizeof(device_map));
    if (d) { d->h = kh_init(dev); }
    return d;
}

void
device_map_destroy(device_map** d_p)
{
    device_map* dmap = *d_p;
    *d_p = NULL;
    for (khint_t k = kh_begin(dmap->h); k != kh_end(dmap->h); ++k) {
        if (kh_exist(dmap->h, k)) {
            device* d = kh_val(dmap->h, k);
            device_destroy(&d);
        }
    }
    kh_destroy(dev, dmap->h);
    linq_free(dmap);
}

device**
device_map_insert(
    device_map* dmap,
    zsock_t** sock_p,
    router* router,
    const char* serial,
    const char* product)
{
    int ret = 0;
    device* d = device_create(sock_p, router, serial, product);
    khiter_t k = kh_put(dev, dmap->h, device_serial(d), &ret);
    linq_assert(ret == 1); // If double insert we crash
    kh_val(dmap->h, k) = d;
    return &kh_val(dmap->h, k);
}

uint32_t
device_map_remove(device_map* dmap, const char* serial)
{
    khiter_t k;
    device* d;
    uint32_t count = 0;
    if (!((k = kh_get(dev, dmap->h, serial)) == kh_end(dmap->h))) {
        d = kh_val(dmap->h, k);
        kh_del(dev, dmap->h, k);
        device_destroy(&d);
        count = 1;
    }
    return count;
}

device**
device_map_get(device_map* dmap, const char* serial)
{
    khiter_t k;
    return ((k = kh_get(dev, dmap->h, serial)) == kh_end(dmap->h))
               ? NULL
               : &kh_val(dmap->h, k);
}

uint32_t
device_map_size(device_map* map)
{
    return kh_size(map->h);
}
