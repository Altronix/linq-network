#include "device.h"

typedef struct device
{
    zsock_t** sock_p;
    uint8_t router[256];
    char serial[64];
    char product[64];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device;

device*
device_create(
    zsock_t** sock_p,
    zframe_t* router,
    zframe_t* serial,
    zframe_t* product)
{
    device* d = linq_malloc(sizeof(device));
    linq_assert(d);
    linq_assert(zframe_size(router) < sizeof(d->router));
    linq_assert(zframe_size(serial) < sizeof(d->serial));
    linq_assert(zframe_size(product) < sizeof(d->product));
    memset(d, 0, sizeof(device));
    d->sock_p = sock_p;
    memcpy(d->router, zframe_data(router), zframe_size(router));
    snprintf(d->serial, sizeof(d->serial), "%s", zframe_data(serial));
    snprintf(d->product, sizeof(d->product), "%s", zframe_data(product));
    return d;
}

void
device_destroy(device** d_p)
{
    device* d = *d_p;
    memset(d, 0, sizeof(device));
    *d_p = NULL;
}

const char*
device_serial(device* d)
{
    return d->serial;
}

int
device_remove_serial(const char* serial)
{
    ((void)serial);
    return 0;
}

int
device_remove_product(const char* product)
{
    ((void)product);
    return 0;
}

uint32_t
device_count()
{
    return 0;
}
