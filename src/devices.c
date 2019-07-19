#include "device.h"
#include "devices.h"

KHASH_MAP_INIT_STR(dev, device_s*);

typedef struct devices_s
{
    kh_dev_t* h;
} devices_s;

devices_s*
devices_create()
{
    devices_s* d = linq_malloc(sizeof(devices_s));
    if (d) { d->h = kh_init(dev); }
    return d;
}

void
devices_destroy(devices_s** d_p)
{
    devices_s* dmap = *d_p;
    *d_p = NULL;
    for (khint_t k = kh_begin(dmap->h); k != kh_end(dmap->h); ++k) {
        if (kh_exist(dmap->h, k)) {
            device_s* d = kh_val(dmap->h, k);
            device_destroy(&d);
        }
    }
    kh_destroy(dev, dmap->h);
    linq_free(dmap);
}

device_s**
devices_insert(
    devices_s* dmap,
    zsock_t** sock_p,
    uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* product)
{
    int ret = 0;
    device_s* d = device_create(sock_p, router, router_sz, serial, product);
    khiter_t k = kh_put(dev, dmap->h, device_serial(d), &ret);
    linq_assert(ret == 1); // If double insert we crash
    kh_val(dmap->h, k) = d;
    return &kh_val(dmap->h, k);
}

uint32_t
devices_remove(devices_s* dmap, const char* serial)
{
    khiter_t k;
    device_s* d;
    uint32_t count = 0;
    if (!((k = kh_get(dev, dmap->h, serial)) == kh_end(dmap->h))) {
        d = kh_val(dmap->h, k);
        kh_del(dev, dmap->h, k);
        device_destroy(&d);
        count = 1;
    }
    return count;
}

device_s**
devices_get(devices_s* dmap, const char* serial)
{
    khiter_t k;
    return ((k = kh_get(dev, dmap->h, serial)) == kh_end(dmap->h))
               ? NULL
               : &kh_val(dmap->h, k);
}

uint32_t
devices_size(devices_s* map)
{
    return kh_size(map->h);
}
