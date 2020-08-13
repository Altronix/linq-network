// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "device.h"
#include "containers.h"

#define exe_on_complete(rp, err, dat, dp)                                      \
    do {                                                                       \
        if ((*rp)->on_complete) (*rp)->on_complete((*rp)->ctx, err, dat, dp);  \
    } while (0)

typedef struct request_zmtp_s
{
    router_s forward;
    uint32_t sent_at;
    uint32_t retry_at;
    uint32_t retry_count;
    void* ctx;
    linq_request_complete_fn on_complete;
    zframe_t* frames[FRAME_REQ_DATA_IDX + 1];
} request_zmtp_s;
static void request_destroy(request_zmtp_s** r_p);
LIST_INIT(request, request_zmtp_s, request_destroy);

// main class struct (extends linq_network_socket_s)
typedef struct device_zmtp_s
{
    zsock_t* sock;   // linq_network_socket_s expects zsock_t* to be first
    router_s router; // linq_network_socket_s expects router to be second
    char serial[SID_LEN];
    request_list_s* requests;
    request_zmtp_s* request_pending;
    char type[TID_LEN];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device_zmtp_s;
MAP_INIT(device, device_zmtp_s, device_destroy);

static zframe_t*
write_path_to_frame(const char* method, const char* path, uint32_t path_len)
{
    ((void)path_len);
    uint32_t sz;
    char url[128];
    if (*path == '/') {
        sz = snprintf(url, sizeof(url), "%s %.*s", method, path_len, path);
    } else {
        sz = snprintf(url, sizeof(url), "%s /%.*s", method, path_len, path);
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

static request_zmtp_s*
request_alloc_mem(
    device_zmtp_s* device,
    E_REQUEST_METHOD method,
    const char* p,
    uint32_t plen,
    const char* d,
    uint32_t dlen,
    linq_request_complete_fn fn,
    void* context)
{
    bool hop = device_hops(device);
    const char* s = device_serial(device);
    uint32_t slen = strlen(s);
    request_zmtp_s* r = linq_network_malloc(sizeof(request_zmtp_s));
    if (r) {
        memset(r, 0, sizeof(request_zmtp_s));
        r->on_complete = fn;
        r->ctx = context;
        r->frames[FRAME_VER_IDX] = zframe_new("\0", 1);
        r->frames[FRAME_TYP_IDX] = zframe_new("\1", 1);
        r->frames[FRAME_SID_IDX] = hop ? zframe_new(s, slen) : NULL;
        r->frames[FRAME_REQ_PATH_IDX] = path_to_frame(method, p, plen);
        r->frames[FRAME_REQ_DATA_IDX] = d && dlen ? zframe_new(d, dlen) : NULL;
        if (!(r->frames[FRAME_VER_IDX] && r->frames[FRAME_TYP_IDX] &&
              ((d && r->frames[FRAME_REQ_DATA_IDX]) || !d) &&
              ((hop && r->frames[FRAME_SID_IDX]) || !hop))) {
            request_destroy(&r);
        }
    }
    return r;
}

static request_zmtp_s*
request_alloc(
    device_zmtp_s* device,
    E_REQUEST_METHOD method,
    const char* path,
    const char* json,
    linq_request_complete_fn on_complete,
    void* context)
{
    return request_alloc_mem(
        device,
        method,
        path,
        strlen(path),
        json,
        json ? strlen(json) : 0,
        on_complete,
        context);
}

static void
request_destroy(request_zmtp_s** r_p)
{
    request_zmtp_s* r = *r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_network_free(r);
}

static void
request_router_id_set(request_zmtp_s* r, uint8_t* rid, uint32_t rid_len)
{
    if (r->frames[FRAME_RID_IDX]) zframe_destroy(&r->frames[FRAME_RID_IDX]);
    r->frames[FRAME_RID_IDX] = zframe_new(rid, rid_len);
    linq_network_assert(r->frames[FRAME_RID_IDX]);
}

static const char*
request_serial_get(request_zmtp_s* r)
{
    return (char*)zframe_data(r->frames[FRAME_SID_IDX]);
}

static int
request_send(request_zmtp_s* r, zsock_t* sock)
{
    zmsg_t* msg = zmsg_new();
    int err, c = r->frames[FRAME_RID_IDX] ? 0 : 1;
    while (c < FRAME_REQ_DATA_IDX) {
        if (r->frames[c]) {
            zframe_t* f = zframe_dup(r->frames[c]);
            linq_network_assert(f);
            zmsg_append(msg, &f);
        }
        c++;
    }
    if (r->frames[c]) zmsg_append(msg, &r->frames[c]);
    r->sent_at = sys_tick();
    err = zmsg_send(&msg, sock);
    if (err) zmsg_destroy(&msg);
    return err;
}

device_zmtp_s*
device_create(
    zsock_t* sock,
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* type)
{
    device_zmtp_s* d = linq_network_malloc(sizeof(device_zmtp_s));
    if (d) {
        memset(d, 0, sizeof(device_zmtp_s));
        d->sock = sock;
        d->requests = request_list_create();
        d->birth = d->last_seen = sys_tick();
        if (router) device_update_router(d, router, router_sz);
        snprintf(d->serial, sizeof(d->serial), "%s", serial);
        snprintf(d->type, sizeof(d->type), "%s", type);
    }
    return d;
}

void
device_destroy(device_zmtp_s** d_p)
{
    device_zmtp_s* d = *d_p;
    while (d->request_pending) {
        device_request_resolve(
            d, LINQ_ERROR_SHUTTING_DOWN, "{\"error\":\"shutting down...\"}");
        d->request_pending = request_list_pop(d->requests);
    }
    request_list_destroy(&d->requests);
    memset(d, 0, sizeof(device_zmtp_s));
    *d_p = NULL;
    linq_network_free(d);
}

const char*
device_serial(device_zmtp_s* d)
{
    return d->serial;
}

const char*
device_type(device_zmtp_s* d)
{
    return d->type;
}

const router_s*
device_router(device_zmtp_s* d)
{
    return &d->router;
}

bool
device_no_hops(device_zmtp_s* d)
{
    // Note that if a device has a router, than that means the device connected
    // to our listener directly (therefore no hops). This is opposed to a
    // device who we discovered via broadcasting heartbeats from remote nodes
    return device_router(d)->sz ? true : false;
}

bool
device_hops(device_zmtp_s* d)
{
    // Note that if a device has a router, than that means the device connected
    // to our listener directly (therefore no hops). This is opposed to a
    // device who we discovered via broadcasting heartbeats from remote nodes
    return !device_no_hops(d);
}

void
device_update_router(device_zmtp_s* d, const uint8_t* rid, uint32_t sz)
{
    memcpy(&d->router.id, rid, sz);
    d->router.sz = sz;
}

uint32_t
device_last_seen(device_zmtp_s* d)
{
    return d->last_seen;
}

uint32_t
device_uptime(device_zmtp_s* d)
{
    return d->last_seen - d->birth;
}

void
device_heartbeat(device_zmtp_s* d)
{
    d->last_seen = sys_tick();
}

E_REQUEST_METHOD
device_method_from_str(const char* method)
{
    uint32_t l = strlen(method);
    if (l == 3) {
        if (!memcmp(method, "GET", l)) {
            return REQUEST_METHOD_GET;
        } else if (!(memcmp(method, "PUT", l))) {
            return REQUEST_METHOD_POST; // TODO support PUT
        }
    } else if (l == 4 && !memcmp(method, "POST", l)) {
        return REQUEST_METHOD_POST;
    } else if (l == 6 && !memcmp(method, "DELETE", l)) {
        return REQUEST_METHOD_DELETE;
    }
    assert(false);
    return -1; // should never return
}

void
device_send(
    device_zmtp_s* d,
    E_REQUEST_METHOD method,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_request_complete_fn fn,
    void* context)
{
    request_zmtp_s* r =
        request_alloc_mem(d, method, path, plen, json, jlen, fn, context);
    if (r) {
        request_list_push(d->requests, &r);
        device_request_flush_w_check(d);
    } else {
        exe_on_complete(&r, d->serial, LINQ_ERROR_OOM, NULL);
    }
}

void
device_send_raw(
    device_zmtp_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    uint32_t plen = strlen(path), jlen = json ? strlen(json) : 0;
    device_send(d, REQUEST_METHOD_RAW, path, plen, json, jlen, fn, context);
}

uint32_t
device_request_sent_at(device_zmtp_s* r)
{
    return r->request_pending ? r->request_pending->sent_at : 0;
}

uint32_t
device_request_retry_count(device_zmtp_s* r)
{
    linq_network_assert(r->request_pending);
    return r->request_pending->retry_count;
}

uint32_t
device_request_retry_at(device_zmtp_s* r)
{
    linq_network_assert(r->request_pending);
    return r->request_pending->retry_at;
}

void
device_request_retry_at_set(device_zmtp_s* r, uint32_t set)
{
    linq_network_assert(r->request_pending);
    r->request_pending->retry_at = set;
}

void
device_request_resolve(device_zmtp_s* d, E_LINQ_ERROR err, const char* str)
{
    char json[JSON_LEN + 1];
    request_zmtp_s** r_p = &d->request_pending;
    snprintf(json, sizeof(json), "%s", str);
    exe_on_complete(r_p, d->serial, err, json);
    request_destroy(r_p);
}

void
device_request_flush(device_zmtp_s* d)
{
    linq_network_assert(d->request_pending == NULL);
    request_zmtp_s** r_p = &d->request_pending;
    *r_p = request_list_pop(d->requests);
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
    } else {
    }
}

void
device_request_retry(device_zmtp_s* d)
{
    linq_network_assert(d->request_pending);
    request_zmtp_s** r_p = &d->request_pending;
    (*r_p)->retry_at = 0;
    (*r_p)->retry_count++;
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
    } else {
    }
}

void
device_request_flush_w_check(device_zmtp_s* d)
{
    if (request_list_size(d->requests) && !d->request_pending) {
        device_request_flush(d);
    }
}

bool
device_request_pending(device_zmtp_s* n)
{
    return n->request_pending ? true : false;
}

uint32_t
device_request_pending_count(device_zmtp_s* d)
{
    return request_list_size(d->requests) + (d->request_pending ? 1 : 0);
}
