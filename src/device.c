#include "device.h"
#include "request.h"

typedef struct device_s
{
    zsock_t** sock_p;
    router_s router;
    request_list_s* requests;
    request_s* request_pending;
    char serial[64];
    char product[64];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device_s;

static void
flush(device_s* d)
{
    ((void)d);
}

device_s*
device_create(
    zsock_t** sock_p,
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* product)
{
    device_s* d = linq_malloc(sizeof(device_s));
    linq_assert(d);
    memset(d, 0, sizeof(device_s));
    d->sock_p = sock_p;
    d->requests = request_list_create();
    d->birth = d->last_seen = sys_tick();
    device_update_router(d, router, router_sz);
    snprintf(d->serial, sizeof(d->serial), "%s", serial);
    snprintf(d->product, sizeof(d->product), "%s", product);
    return d;
}

void
device_destroy(device_s** d_p)
{
    device_s* d = *d_p;
    request_list_destroy(&d->requests);
    if (d->request_pending) request_destroy(&d->request_pending);
    memset(d, 0, sizeof(device_s));
    *d_p = NULL;
    linq_free(d);
}

const char*
device_serial(device_s* d)
{
    return d->serial;
}

const char*
device_product(device_s* d)
{
    return d->product;
}

const router_s*
device_router(device_s* d)
{
    return &d->router;
}

void
device_update_router(device_s* d, const uint8_t* rid, uint32_t sz)
{
    memcpy(&d->router, rid, sz);
    d->router.sz = sz;
}

uint32_t
device_last_seen(device_s* d)
{
    return d->last_seen;
}

uint32_t
device_uptime(device_s* d)
{
    return d->last_seen - d->birth;
}

void
device_heartbeat(device_s* d)
{
    d->last_seen = sys_tick();
}

void
device_send(device_s* d, request_s** r)
{
    request_list_push(d->requests, r);
    if (!d->request_pending) flush(d);
}

void
device_send_delete(device_s* d, const char* path, linq_request_complete_fn fn)
{
    ((void)d);
    ((void)path);
    ((void)fn);
    // TODO
}

void
device_send_get(device_s* d, const char* path, linq_request_complete_fn fn)
{
    request_s* r = request_create( //
        REQUEST_METHOD_GET,
        device_serial(d),
        path,
        NULL,
        fn);
    if (r) {
        device_send(d, &r);
    } else {
        fn(LINQ_ERROR_OOM, NULL, &d);
    }
}

void
device_send_post(
    device_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn)
{
    ((void)d);
    ((void)path);
    ((void)json);
    ((void)fn);
    // TODO
}

