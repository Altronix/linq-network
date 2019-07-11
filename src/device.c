#include "device.h"

typedef struct device
{
    zsock_t** sock_p;
    router router;
    char serial[64];
    char product[64];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device;

device*
device_create(
    zsock_t** sock_p,
    router* rid,
    const char* serial,
    const char* product)
{
    device* d = linq_malloc(sizeof(device));
    linq_assert(d);
    memset(d, 0, sizeof(device));
    d->sock_p = sock_p;
    memcpy(&d->router, rid, sizeof(router));
    snprintf(d->serial, sizeof(d->serial), "%s", serial);
    snprintf(d->product, sizeof(d->product), "%s", product);
    return d;
}

void
device_destroy(device** d_p)
{
    device* d = *d_p;
    memset(d, 0, sizeof(device));
    *d_p = NULL;
    linq_free(d);
}

const char*
device_serial(device* d)
{
    return d->serial;
}

const char*
device_product(device* d)
{
    return d->product;
}

const router*
device_router(device* d)
{
    return &d->router;
}

void
device_update_router(device* d, router* rid)
{
    memcpy(&d->router, rid, sizeof(router));
}
