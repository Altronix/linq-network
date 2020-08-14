// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "device.h"
#include "containers.h"
#include "log.h"

#define exe_on_complete(rp, err, dat, dp)                                      \
    do {                                                                       \
        if ((*rp)->callback) (*rp)->callback((*rp)->ctx, err, dat, dp);        \
    } while (0)

typedef struct request_zmtp_s
{
    request_s base;
    router_s forward;
    zframe_t* frames[FRAME_REQ_DATA_IDX + 1];
} request_zmtp_s;
static void request_destroy(request_s** r_p);
LIST_INIT(request, request_s, request_destroy);

// main class struct (extends linq_network_socket_s)
typedef struct device_zmtp_s
{
    node_s base;     // will cast into netw_socket_s ...must be on top
    zsock_t* sock;   // will cast into netw_socket_s ...must be on top
    router_s router; // will cast into netw_socket_s ...must be on top
    request_list_s* requests;
} device_zmtp_s;
MAP_INIT(device, node_s, device_destroy);

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

static request_s*
request_alloc_mem(
    node_s* base,
    E_REQUEST_METHOD method,
    const char* p,
    uint32_t plen,
    const char* d,
    uint32_t dlen,
    linq_request_complete_fn fn,
    void* context)
{
    device_zmtp_s* device = (device_zmtp_s*)base;
    bool hop = device_hops(&device->base);
    const char* s = device_serial(&device->base);
    uint32_t slen = strlen(s);
    request_zmtp_s* r = linq_network_malloc(sizeof(request_zmtp_s));
    if (r) {
        memset(r, 0, sizeof(request_zmtp_s));
        r->base.callback = fn;
        r->base.ctx = context;
        r->frames[FRAME_VER_IDX] = zframe_new("\0", 1);
        r->frames[FRAME_TYP_IDX] = zframe_new("\1", 1);
        r->frames[FRAME_SID_IDX] = hop ? zframe_new(s, slen) : NULL;
        r->frames[FRAME_REQ_PATH_IDX] = path_to_frame(method, p, plen);
        r->frames[FRAME_REQ_DATA_IDX] = d && dlen ? zframe_new(d, dlen) : NULL;
        if (!(r->frames[FRAME_VER_IDX] && r->frames[FRAME_TYP_IDX] &&
              ((d && r->frames[FRAME_REQ_DATA_IDX]) || !d) &&
              ((hop && r->frames[FRAME_SID_IDX]) || !hop))) {
            request_destroy((request_s**)&r);
        }
    }
    return (request_s*)r;
}

static request_s*
request_alloc(
    node_s* device,
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
request_destroy(request_s** r_p)
{
    request_zmtp_s* r = *(request_zmtp_s**)r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_network_free(r);
}

static void
request_router_id_set(request_s* base, uint8_t* rid, uint32_t rid_len)
{
    request_zmtp_s* r = (request_zmtp_s*)base;
    if (r->frames[FRAME_RID_IDX]) zframe_destroy(&r->frames[FRAME_RID_IDX]);
    r->frames[FRAME_RID_IDX] = zframe_new(rid, rid_len);
    linq_network_assert(r->frames[FRAME_RID_IDX]);
}

static const char*
request_serial_get(request_s* base)
{
    request_zmtp_s* r = (request_zmtp_s*)base;
    return (char*)zframe_data(r->frames[FRAME_SID_IDX]);
}

static int
request_send(request_s* base, zsock_t* sock)
{
    request_zmtp_s* r = (request_zmtp_s*)base;
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
    r->base.sent_at = sys_tick();
    err = zmsg_send(&msg, sock);
    if (err) zmsg_destroy(&msg);
    return err;
}

node_s*
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
        if (router) device_update_router(&d->base, router, router_sz);
        d->sock = sock;
        d->requests = request_list_create();
        d->base.birth = d->base.last_seen = sys_tick();
        d->base.transport = TRANSPORT_ZMTP;
        snprintf(d->base.serial, sizeof(d->base.serial), "%s", serial);
        snprintf(d->base.type, sizeof(d->base.type), "%s", type);
    }
    return (node_s*)d;
}

void
device_destroy(node_s** base)
{
    device_zmtp_s* d = *(device_zmtp_s**)base;
    while (d->base.pending) {
        device_request_resolve(
            &d->base,
            LINQ_ERROR_SHUTTING_DOWN,
            "{\"error\":\"shutting down...\"}");
        d->base.pending = request_list_pop(d->requests);
    }
    request_list_destroy(&d->requests);
    memset(d, 0, sizeof(device_zmtp_s));
    *base = NULL;
    linq_network_free(d);
}

const char*
device_serial(node_s* d)
{
    return d->serial;
}

const char*
device_type(node_s* d)
{
    return d->type;
}

const router_s*
device_router(node_s* base)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    return &d->router;
}

bool
device_no_hops(node_s* d)
{
    // Note that if a device has a router, than that means the device connected
    // to our listener directly (therefore no hops). This is opposed to a
    // device who we discovered via broadcasting heartbeats from remote nodes
    return device_router(d)->sz ? true : false;
}

bool
device_hops(node_s* d)
{
    // Note that if a device has a router, than that means the device connected
    // to our listener directly (therefore no hops). This is opposed to a
    // device who we discovered via broadcasting heartbeats from remote nodes
    return !device_no_hops(d);
}

void
device_update_router(node_s* base, const uint8_t* rid, uint32_t sz)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    memcpy(&d->router.id, rid, sz);
    d->router.sz = sz;
}

uint32_t
device_last_seen(node_s* d)
{
    return d->last_seen;
}

uint32_t
device_uptime(node_s* d)
{
    return d->last_seen - d->birth;
}

void
device_heartbeat(node_s* d)
{
    d->last_seen = sys_tick();
}

void
device_send(
    node_s* base,
    E_REQUEST_METHOD method,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_request_complete_fn fn,
    void* context)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    request_s* r = request_alloc_mem(
        &d->base, method, path, plen, json, jlen, fn, context);
    if (r) {
        request_list_push(d->requests, &r);
        device_request_flush_w_check(&d->base);
    } else {
        exe_on_complete(&r, d->base.serial, LINQ_ERROR_OOM, NULL);
    }
}

void
device_send_raw(
    node_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    uint32_t plen = strlen(path), jlen = json ? strlen(json) : 0;
    device_send(d, REQUEST_METHOD_RAW, path, plen, json, jlen, fn, context);
}

uint32_t
device_request_sent_at(node_s* r)
{
    return r->pending ? r->pending->sent_at : 0;
}

uint32_t
device_request_retry_count(node_s* base)
{
    linq_network_assert(base->pending);
    return base->pending->retry_count;
}

uint32_t
device_request_retry_at(node_s* base)
{
    linq_network_assert(base->pending);
    return base->pending->retry_at;
}

void
device_request_retry_at_set(node_s* base, uint32_t set)
{
    linq_network_assert(base->pending);
    base->pending->retry_at = set;
}

void
device_request_resolve(node_s* base, E_LINQ_ERROR err, const char* str)
{
    char json[JSON_LEN + 1];
    request_s** r_p = &base->pending;
    snprintf(json, sizeof(json), "%s", str);
    exe_on_complete(r_p, base->serial, err, json);
    request_destroy(r_p);
}

void
device_request_flush(node_s* base)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    linq_network_assert(d->base.pending == NULL);
    request_s** r_p = &d->base.pending;
    *r_p = request_list_pop(d->requests);
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->base.serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
    } else {
    }
}

void
device_request_retry(node_s* base)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    linq_network_assert(d->base.pending);
    request_s** r_p = &d->base.pending;
    (*r_p)->retry_at = 0;
    (*r_p)->retry_count++;
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->base.serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
    } else {
    }
}

void
device_request_flush_w_check(node_s* base)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    if (request_list_size(d->requests) && !d->base.pending) {
        device_request_flush(&d->base);
    }
}

bool
device_request_pending(node_s* n)
{
    return n->pending ? true : false;
}

uint32_t
device_request_pending_count(node_s* base)
{
    device_zmtp_s* d = (device_zmtp_s*)base;
    return request_list_size(d->requests) + (d->base.pending ? 1 : 0);
}

// A socket is closing. Remove all our nodes that reference this socket
uint32_t
device_foreach_remove_if_sock_eq(device_map_s* hash, zsock_t* z)
{
    uint32_t n = 0;
    map_iter iter;
    node_s* v;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter) && (v = map_val(hash, iter))) {
            if (v->transport == TRANSPORT_ZMTP) {
                netw_socket_s* s = (netw_socket_s*)v;
                if (s->sock == z) {
                    device_map_remove(hash, v->serial);
                    n++;
                }
            }
        }
    }
    return n;
}

void
device_foreach_check_request(device_map_s* hash)
{
    node_s* base;
    map_iter iter;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter) && (base = map_val(hash, iter))) {
            if (base->transport == TRANSPORT_ZMTP) {
                device_zmtp_s* d = (device_zmtp_s*)base;
                if (device_request_pending(&d->base)) {
                    uint32_t tick = sys_tick();
                    uint32_t retry_at = device_request_retry_at(&d->base);
                    if (device_request_sent_at(base) + 10000 <= tick) {
                        log_info("(DEVICE) timeout [%s]", d->base.serial);
                        device_request_resolve(
                            &d->base,
                            LINQ_ERROR_TIMEOUT,
                            "{\"error\":\"timeout\"}");
                        device_request_flush_w_check(&d->base);
                    } else if (retry_at && retry_at <= tick) {
                        log_info("(DEVICE) retry [%s]", d->base.serial);
                        device_request_retry(&d->base);
                    }
                }
            }
        }
    }
}
