#include "node.h"
#include "request.h"
#include "requests.h"

#define exe_on_complete(rp, err, dat, dp)                                      \
    do {                                                                       \
        if ((*rp)->on_complete) (*rp)->on_complete((*rp)->ctx, err, dat, dp);  \
    } while (0)

typedef struct node_s
{
    zsock_t** sock_p;
    router_s router;
    requests_s* requests;
    request_s* request_pending;
    char serial[64];
    char type[64];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} node_s;

// Push another request onto socket...
// TODO caller must check request is in queue... refactor check here(?)...
static void
flush(node_s* d)
{
    linq_assert(d->request_pending == NULL);
    request_s** r_p = &d->request_pending;
    *r_p = requests_pop(d->requests);
    request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, *d->sock_p) < 0) {
        exe_on_complete(r_p, LINQ_ERROR_IO, NULL, &d);
        request_destroy(r_p);
    } else {
    }
}

node_s*
node_create(
    zsock_t** sock_p,
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* type)
{
    node_s* d = linq_malloc(sizeof(node_s));
    if (d) {
        memset(d, 0, sizeof(node_s));
        d->sock_p = sock_p;
        d->requests = requests_create();
        d->birth = d->last_seen = sys_tick();
        node_update_router(d, router, router_sz);
        snprintf(d->serial, sizeof(d->serial), "%s", serial);
        snprintf(d->type, sizeof(d->type), "%s", type);
    }
    return d;
}

void
node_destroy(node_s** d_p)
{
    node_s* d = *d_p;
    requests_destroy(&d->requests);
    if (d->request_pending) request_destroy(&d->request_pending);
    memset(d, 0, sizeof(node_s));
    *d_p = NULL;
    linq_free(d);
}

zsock_t**
node_socket(node_s* n)
{
    return n->sock_p;
}

const char*
node_serial(node_s* d)
{
    return d->serial;
}

const char*
node_type(node_s* d)
{
    return d->type;
}

const router_s*
node_router(node_s* d)
{
    return &d->router;
}

void
node_update_router(node_s* d, const uint8_t* rid, uint32_t sz)
{
    memcpy(&d->router.id, rid, sz);
    d->router.sz = sz;
}

uint32_t
node_last_seen(node_s* d)
{
    return d->last_seen;
}

uint32_t
node_uptime(node_s* d)
{
    return d->last_seen - d->birth;
}

void
node_heartbeat(node_s* d)
{
    d->last_seen = sys_tick();
}

void
node_send_frames(node_s* d, frames_s* forward)
{
    uint32_t count = 0;
    zmsg_t* msg = zmsg_new();
    if (msg) {
        zframe_t* router = zframe_new(d->router.id, d->router.sz);
        if (router) {
            zmsg_append(msg, &router);
            for (uint32_t i = 0; i < forward->n; i++) {
                zframe_t* frame = zframe_dup(forward->frames[i]);
                if (!frame) break;
                count++;
                zmsg_append(msg, &frame);
            }
            if (count == forward->n) {
                int err = zmsg_send(&msg, *d->sock_p);
                if (err) zmsg_destroy(&msg);
            }
        }
    }
}

void
node_send(node_s* d, request_s** r)
{
    requests_push(d->requests, r);
    if (!d->request_pending && requests_size(d->requests)) flush(d);
}

static void
send_method(
    node_s* d,
    E_REQUEST_METHOD method,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    request_s* r =
        request_create(method, node_serial(d), path, json, fn, context);
    if (r) {
        node_send(d, &r);
    } else {
        exe_on_complete(&r, LINQ_ERROR_OOM, NULL, &d);
    }
}

void
node_send_delete(
    node_s* d,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_DELETE, path, NULL, fn, context);
}

void
node_send_get(
    node_s* d,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_GET, path, NULL, fn, context);
}

void
node_send_post(
    node_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_POST, path, json, fn, context);
}

void
node_resolve_request(node_s* d, E_LINQ_ERROR err, const char* str)
{
    char json[JSON_LEN + 1];
    request_s** r_p = &d->request_pending;
    snprintf(json, sizeof(json), "%s", str);
    exe_on_complete(r_p, err, json, &d);
    request_destroy(r_p);
    if (requests_size(d->requests)) flush(d);
}

request_s*
node_request_pending(node_s* n)
{
    return n->request_pending;
}

uint32_t
node_request_pending_count(node_s* d)
{
    return requests_size(d->requests) + (d->request_pending ? 1 : 0);
}
