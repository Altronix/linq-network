#include "device.h"
#include "containers.h"

#define exe_on_complete(rp, err, dat, dp)                                      \
    do {                                                                       \
        if ((*rp)->on_complete) (*rp)->on_complete((*rp)->ctx, err, dat, dp);  \
    } while (0)

typedef struct request_s request_s;

static void request_destroy(request_s** r_p);
LIST_INIT(requests, request_s, request_destroy);

typedef enum E_REQUEST_METHOD
{
    REQUEST_METHOD_RAW = 0,
    REQUEST_METHOD_GET,
    REQUEST_METHOD_POST,
    REQUEST_METHOD_DELETE
} E_REQUEST_METHOD;

typedef struct request_s
{
    router_s forward;
    uint32_t sent_at;
    void* ctx;
    linq_request_complete_fn on_complete;
    zframe_t* frames[FRAME_REQ_DATA_IDX + 1];
} request_s;

typedef struct device_s
{
    zsock_t* sock;
    router_s router;
    list_requests_s* requests;
    request_s* request_pending;
    char serial[SID_LEN];
    char type[TID_LEN];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device_s;

static zframe_t*
write_path_to_frame(const char* method, const char* path, uint32_t path_len)
{
    ((void)path_len);
    uint32_t sz;
    char url[128];
    if (*path == '/') {
        sz = snprintf(url, sizeof(url), "%s %s", method, path);
    } else {
        sz = snprintf(url, sizeof(url), "%s /%s", method, path);
    }
    return zframe_new(url, sz);
}

static zframe_t*
path_to_frame(E_REQUEST_METHOD method, const char* path, uint32_t path_len)
{
    zframe_t* frame = NULL;

    switch (method) {
        case REQUEST_METHOD_RAW: frame = zframe_new(path, path_len); break;
        case REQUEST_METHOD_GET:
            frame = write_path_to_frame("GET", path, path_len);
            break;
        case REQUEST_METHOD_POST:
            frame = write_path_to_frame("POST", path, path_len);
            break;
        case REQUEST_METHOD_DELETE:
            frame = write_path_to_frame("DELETE", path, path_len);
            break;
    }
    return frame;
}

static request_s*
request_alloc_mem(
    E_REQUEST_METHOD method,
    const char* s,
    uint32_t slen,
    const char* p,
    uint32_t plen,
    const char* d,
    uint32_t dlen,
    linq_request_complete_fn fn,
    void* context)
{
    request_s* r = linq_malloc(sizeof(request_s));
    if (r) {
        memset(r, 0, sizeof(request_s));
        r->on_complete = fn;
        r->ctx = context;
        r->frames[FRAME_VER_IDX] = zframe_new("\0", 1);
        r->frames[FRAME_TYP_IDX] = zframe_new("\1", 1);
        r->frames[FRAME_SID_IDX] = zframe_new(s, slen);
        r->frames[FRAME_REQ_PATH_IDX] = path_to_frame(method, p, plen);
        r->frames[FRAME_REQ_DATA_IDX] = d && dlen ? zframe_new(d, dlen) : NULL;
        if (!(r->frames[FRAME_VER_IDX] && r->frames[FRAME_TYP_IDX] &&
              r->frames[FRAME_SID_IDX] && r->frames[FRAME_REQ_PATH_IDX] &&
              ((d && r->frames[FRAME_REQ_DATA_IDX]) || !d))) {
            request_destroy(&r);
        }
    }
    return r;
}

static request_s*
request_alloc(
    E_REQUEST_METHOD method,
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn on_complete,
    void* context)
{
    return request_alloc_mem(
        method,
        serial,
        strlen(serial),
        path,
        strlen(path),
        json,
        json ? strlen(json) : 0,
        on_complete,
        context);
}

static void
request_destroy(request_s** r_p)
{
    request_s* r = *r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_free(r);
}

static void
request_router_id_set(request_s* r, uint8_t* rid, uint32_t rid_len)
{
    r->frames[FRAME_RID_IDX] = zframe_new(rid, rid_len);
    linq_assert(r->frames[FRAME_RID_IDX]);
}

static const char*
request_serial_get(request_s* r)
{
    return (char*)zframe_data(r->frames[FRAME_SID_IDX]);
}

static int
request_send(request_s* r, zsock_t* sock)
{
    zmsg_t* msg = zmsg_new();
    int err, c = 0;
    while (c < FRAME_REQ_DATA_IDX) zmsg_append(msg, &r->frames[c++]);
    if (r->frames[c]) zmsg_append(msg, &r->frames[c]);
    r->sent_at = sys_tick();
    err = zmsg_send(&msg, sock);
    if (err) zmsg_destroy(&msg);
    return err;
}

// Push another request onto socket...
// TODO caller must check request is in queue... refactor check here(?)...
static void
flush(device_s* d)
{
    linq_assert(d->request_pending == NULL);
    request_s** r_p = &d->request_pending;
    *r_p = list_requests_pop(d->requests);
    request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, LINQ_ERROR_IO, NULL, &d);
        request_destroy(r_p);
    } else {
    }
}

device_s*
device_create(
    zsock_t* sock,
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* type)
{
    device_s* d = linq_malloc(sizeof(device_s));
    if (d) {
        memset(d, 0, sizeof(device_s));
        d->sock = sock;
        d->requests = list_requests_create();
        d->birth = d->last_seen = sys_tick();
        device_update_router(d, router, router_sz);
        snprintf(d->serial, sizeof(d->serial), "%s", serial);
        snprintf(d->type, sizeof(d->type), "%s", type);
    }
    return d;
}

void
device_destroy(device_s** d_p)
{
    device_s* d = *d_p;
    list_requests_destroy(&d->requests);
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
device_type(device_s* d)
{
    return d->type;
}

const router_s*
device_router(device_s* d)
{
    return &d->router;
}

zsock_t*
device_socket(device_s* d)
{
    return d->sock;
}

void
device_update_router(device_s* d, const uint8_t* rid, uint32_t sz)
{
    memcpy(&d->router.id, rid, sz);
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

static void
send_method(
    device_s* d,
    E_REQUEST_METHOD method,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    request_s* r =
        request_alloc(method, device_serial(d), path, json, fn, context);
    if (r) {
        list_requests_push(d->requests, &r);
        if (!d->request_pending && list_requests_size(d->requests)) flush(d);
    } else {
        exe_on_complete(&r, LINQ_ERROR_OOM, NULL, &d);
    }
}

void
device_send_delete(
    device_s* d,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_DELETE, path, NULL, fn, context);
}

void
device_send_get(
    device_s* d,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_GET, path, NULL, fn, context);
}

void
device_send_post(
    device_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_POST, path, json, fn, context);
}

void
device_send(
    device_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    send_method(d, REQUEST_METHOD_RAW, path, json, fn, context);
}

uint32_t
device_request_sent_at(device_s* r)
{
    return r->request_pending ? r->request_pending->sent_at : 0;
}

void
device_request_resolve(device_s* d, E_LINQ_ERROR err, const char* str)
{
    char json[JSON_LEN + 1];
    request_s** r_p = &d->request_pending;
    snprintf(json, sizeof(json), "%s", str);
    exe_on_complete(r_p, err, json, &d);
    request_destroy(r_p);
    if (list_requests_size(d->requests)) flush(d);
}

bool
device_request_pending(device_s* n)
{
    return n->request_pending ? true : false;
}

uint32_t
device_request_pending_count(device_s* d)
{
    return list_requests_size(d->requests) + (d->request_pending ? 1 : 0);
}
