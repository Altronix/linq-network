#include "device_map.h"

KHASH_MAP_INIT_STR(dev, device*);

typedef struct device_map
{
    kh_dev_t* device_map;
    uint32_t count;
} device_map;

device_map*
device_map_create()
{
    device_map* d = linq_malloc(sizeof(device_map));
    if (d) {
        d->device_map = kh_init(dev);
        d->count = 0;
    }
    return d;
}

void
device_map_destroy(device_map** d_p)
{
    device_map* d = *d_p;
    *d_p = NULL;
    kh_destroy(dev, d->device_map);
    linq_free(d);
}

void
device_map_add(
    zsock_t** sock_p,
    zframe_t* router,
    zframe_t* serial,
    zframe_t* product)
{
    ((void)sock_p);
    ((void)router);
    ((void)serial);
    ((void)product);
}

