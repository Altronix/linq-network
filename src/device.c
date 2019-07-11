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
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* product)
{
    device* d = linq_malloc(sizeof(device));
    linq_assert(d);
    memset(d, 0, sizeof(device));
    d->sock_p = sock_p;
    d->birth = d->last_seen = sys_tick();
    device_update_router(d, router, router_sz);
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

void
device_heartbeat(device* d)
{
    d->last_seen = sys_tick();
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
device_update_router(device* d, const uint8_t* rid, uint32_t sz)
{
    memcpy(&d->router, rid, sz);
    d->router.sz = sz;
}

uint32_t
device_last_seen(device* d)
{
    return d->last_seen;
}

uint32_t
device_uptime(device* d)
{
    return d->last_seen - d->birth;
}

