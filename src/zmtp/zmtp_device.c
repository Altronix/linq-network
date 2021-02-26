// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zmtp_device.h"
#include "common/containers.h"
#include "common/device.h"
#include "common/log.h"
#include "common/sys/sys.h"

#define dev_info(...) log_info("DEV", __VA_ARGS__)
#define dev_warn(...) log_warn("DEV", __VA_ARGS__)
#define dev_debug(...) log_debug("DEV", __VA_ARGS__)
#define dev_trace(...) log_trace("DEV", __VA_ARGS__)
#define dev_error(...) log_error("DEV", __VA_ARGS__)
#define dev_fatal(...) log_fatal("DEV", __VA_ARGS__)

#define exe_on_complete(rp, err, dat, dp)                                      \
    do {                                                                       \
        if ((*rp)->callback) (*rp)->callback((*rp)->ctx, err, dat, dp);        \
    } while (0)

typedef struct
{
    uint32_t sz;
    uint8_t data[];
} packet_s;

typedef struct request_zmtp_s
{
    request_s base;
    router_s forward;
    packet_s* frames[FRAME_REQ_DATA_IDX + 1];
} request_zmtp_s;
static void request_destroy(request_s** r_p);
LIST_INIT(request, request_s, request_destroy);

static int retry_timeout = LINQ_NETW_RETRY_TIMEOUT;
static int max_retry = LINQ_NETW_MAX_RETRY;

// main class struct (extends linq_network_socket_s)
typedef struct zmtp_device_s
{
    node_s base;        // will cast into netw_socket_s ...must be on top
    zmq_socket_s* sock; // will cast into netw_socket_s ...must be on top
    router_s router;    // will cast into netw_socket_s ...must be on top
    request_list_s* requests;
} zmtp_device_s;

static packet_s*
write_frame(const char* data, uint32_t len)
{
    packet_s* p = linq_network_malloc(sizeof(packet_s) + len);
    linq_network_assert(p);
    p->sz = len;
    memcpy(p->data, data, len);
    return p;
}

static packet_s*
write_path_to_frame(const char* method, const char* path, uint32_t path_len)
{
    uint32_t sz;
    packet_s* pack = NULL;
    if (*path == '/') {
        sz = strlen(method) + 1 + path_len;
        pack = linq_network_malloc(sizeof(packet_s) + sz + 1);
        linq_network_assert(pack);
        pack->sz = sz;
        snprintf((char*)pack->data, sz + 1, "%s %.*s", method, path_len, path);
    } else {
        sz = strlen(method) + 2 + path_len;
        pack = linq_network_malloc(sizeof(packet_s) + sz + 1);
        linq_network_assert(pack);
        pack->sz = sz;
        snprintf((char*)pack->data, sz + 1, "%s /%.*s", method, path_len, path);
    }
    return pack;
}

static packet_s*
path_to_frame(E_REQUEST_METHOD method, const char* path, uint32_t path_len)
{
    packet_s* frame = NULL;

    switch (method) {
        case REQUEST_METHOD_RAW: frame = write_frame(path, path_len); break;
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
    // When sending a request to a device we don't include the serial number
    zmtp_device_s* device = (zmtp_device_s*)base;
    const char* s = device_serial(&device->base);
    uint32_t slen = strlen(s);
    request_zmtp_s* r = linq_network_malloc(sizeof(request_zmtp_s));
    if (r) {
        memset(r, 0, sizeof(request_zmtp_s));
        r->base.callback = fn;
        r->base.ctx = context;
        r->base.retry_at = sys_tick() + retry_timeout;
        r->frames[FRAME_VER_IDX] = write_frame("\0", 1);
        r->frames[FRAME_TYP_IDX] = write_frame("\1", 1);
        r->frames[FRAME_SID_IDX] = write_frame(s, slen);
        r->frames[FRAME_REQ_PATH_IDX] = path_to_frame(method, p, plen);
        r->frames[FRAME_REQ_DATA_IDX] = d && dlen ? write_frame(d, dlen) : NULL;
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
    for (uint32_t i = 0; i < FRAME_REQ_DATA_IDX + 1; i++) {
        if (r->frames[i]) {
            linq_network_free(r->frames[i]);
            r->frames[i] = NULL;
        }
    }
    linq_network_free(r);
}

static void
request_router_id_set(request_s* base, uint8_t* rid, uint32_t rid_len)
{
    request_zmtp_s* r = (request_zmtp_s*)base;
    if (!r->frames[FRAME_RID_IDX]) {
        r->frames[FRAME_RID_IDX] = write_frame((char*)rid, rid_len);
    } else if (!((r->frames[FRAME_RID_IDX]->sz == rid_len) &&
                 (!memcmp(r->frames[FRAME_RID_IDX]->data, rid, rid_len)))) {
        linq_network_free(r->frames[FRAME_RID_IDX]);
        r->frames[FRAME_RID_IDX] = write_frame((char*)rid, rid_len);
    }
}

static int
request_send(request_s* base, zmq_socket_s* sock)
{
    dev_trace("request_send()");
    request_zmtp_s* r = (request_zmtp_s*)base;
    zmq_msg_t msgs[6];
    int err, flags[6] = {
        ZMQ_SNDMORE,                                     // router
        ZMQ_SNDMORE,                                     // version
        ZMQ_SNDMORE,                                     // type
        ZMQ_SNDMORE,                                     // serial
        r->frames[FRAME_REQ_DATA_IDX] ? ZMQ_SNDMORE : 0, // path
        0                                                // [,dat]
    };
    int c = r->frames[FRAME_RID_IDX] ? 0 : 1;
    for (; c <= 5 && r->frames[c]; c++) {
        err = zmq_msg_init_size(&msgs[c], r->frames[c]->sz);
        linq_network_assert(err == 0);
        memcpy(zmq_msg_data(&msgs[c]), r->frames[c]->data, r->frames[c]->sz);
        err = zmq_msg_send(&msgs[c], sock, flags[c]);
        if (err < 0) {
            dev_error("Failed to send to socket!");
            zmq_msg_close(&msgs[c]);
            break;
        }
    }
    r->base.sent_at = sys_tick();
    return err;
}

node_s*
zmtp_device_create(
    zmq_socket_s* sock,
    const uint8_t* router,
    uint32_t router_sz,
    const char* serial,
    const char* type)
{
    zmtp_device_s* d = linq_network_malloc(sizeof(zmtp_device_s));
    if (d) {
        memset(d, 0, sizeof(zmtp_device_s));
        if (router) zmtp_device_update_router(&d->base, router, router_sz);
        d->sock = sock;
        d->requests = request_list_create();
        d->base.birth = d->base.last_seen = sys_tick();
        d->base.transport = TRANSPORT_ZMTP;
        d->base.poll = zmtp_device_poll;
        d->base.free = zmtp_device_destroy;
        d->base.send = zmtp_device_send;
        snprintf(d->base.serial, sizeof(d->base.serial), "%s", serial);
        snprintf(d->base.type, sizeof(d->base.type), "%s", type);
    }
    return (node_s*)d;
}

void
zmtp_device_destroy(node_s** base)
{
    zmtp_device_s* d = *(zmtp_device_s**)base;
    while (d->base.pending) {
        zmtp_device_request_resolve(
            &d->base,
            LINQ_ERROR_SHUTTING_DOWN,
            "{\"error\":\"shutting down...\"}");
        d->base.pending = request_list_pop(d->requests);
    }
    request_list_destroy(&d->requests);
    memset(d, 0, sizeof(zmtp_device_s));
    *base = NULL;
    linq_network_free(d);
}

const router_s*
zmtp_device_router(node_s* base)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    return &d->router;
}

void
zmtp_device_update_router(node_s* base, const uint8_t* rid, uint32_t sz)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    memcpy(&d->router.id, rid, sz);
    d->router.sz = sz;
}

void
zmtp_device_send(
    node_s* base,
    E_REQUEST_METHOD method,
    const char* path,
    uint32_t plen,
    const char* json,
    uint32_t jlen,
    linq_request_complete_fn fn,
    void* context)
{
    int e;
    zmtp_device_s* d = (zmtp_device_s*)base;
    request_s* r = request_alloc_mem(
        &d->base, method, path, plen, json, jlen, fn, context);
    if (r) {
        request_list_push(d->requests, &r);
        zmtp_device_request_flush_w_check(&d->base);
    } else {
        e = json && !jlen ? LINQ_ERROR_BAD_ARGS : LINQ_ERROR_OOM;
        if (fn) fn(context, d->base.serial, e, NULL);
    }
}

void
zmtp_device_send_raw(
    node_s* d,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    uint32_t plen = strlen(path), jlen = json ? strlen(json) : 0;
    zmtp_device_send(
        d, REQUEST_METHOD_RAW, path, plen, json, jlen, fn, context);
}

uint32_t
zmtp_device_request_sent_at(node_s* r)
{
    return r->pending ? r->pending->sent_at : 0;
}

uint32_t
zmtp_device_request_retry_count(node_s* base)
{
    linq_network_assert(base->pending);
    return base->pending->retry_count;
}

uint32_t
zmtp_device_request_retry_at(node_s* base)
{
    linq_network_assert(base->pending);
    return base->pending->retry_at;
}

void
zmtp_device_request_retry_at_set(node_s* base, int set)
{
    linq_network_assert(base->pending);
    base->pending->retry_at = set >= 0 ? set : sys_tick() + retry_timeout;
    ;
}

void
zmtp_device_request_resolve(node_s* base, E_LINQ_ERROR err, const char* str)
{
    char json[JSON_LEN + 1];
    request_s** r_p = &base->pending;
    snprintf(json, sizeof(json), "%s", str);
    exe_on_complete(r_p, base->serial, err, json);
    request_destroy(r_p);
}

void
zmtp_device_request_flush(node_s* base)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    linq_network_assert(d->base.pending == NULL);
    request_s** r_p = &d->base.pending;
    *r_p = request_list_pop(d->requests);
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->base.serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
        dev_trace("[%.6s...] send fail", device_serial(base));
    } else {
        dev_trace("[%.6s...] send sent!", device_serial(base));
    }
}

void
zmtp_device_request_retry(node_s* base)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    linq_network_assert(d->base.pending);
    request_s** r_p = &d->base.pending;
    (*r_p)->retry_at = sys_tick() + retry_timeout;
    (*r_p)->retry_count++;
    if (d->router.sz) request_router_id_set(*r_p, d->router.id, d->router.sz);
    if (request_send(*r_p, d->sock) < 0) {
        exe_on_complete(r_p, d->base.serial, LINQ_ERROR_IO, NULL);
        request_destroy(r_p);
        dev_trace("[%.6s...] retry fail", device_serial(base));
    } else {
        dev_trace("[%.6s...] retry sent!", device_serial(base));
    }
}

void
zmtp_device_request_flush_w_check(node_s* base)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    if (request_list_size(d->requests) && !d->base.pending) {
        zmtp_device_request_flush(&d->base);
    }
}

bool
zmtp_device_request_pending(node_s* n)
{
    return n->pending ? true : false;
}

uint32_t
zmtp_device_request_pending_count(node_s* base)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    return request_list_size(d->requests) + (d->base.pending ? 1 : 0);
}

void
zmtp_device_poll(node_s* base, void* ctx)
{
    zmtp_device_s* d = (zmtp_device_s*)base;
    if (zmtp_device_request_pending(&d->base)) {
        uint32_t tick = sys_tick();
        uint32_t retry_at = zmtp_device_request_retry_at(&d->base);
        uint32_t retry_count = zmtp_device_request_retry_count(&d->base);
        if (tick >= retry_at) {
            dev_info("timeout [%s]", d->base.serial);
            if (!(retry_count >= max_retry)) {
                dev_info("retry (%d) [%s]", retry_count, d->base.serial);
                zmtp_device_request_retry(&d->base);
            } else {
                zmtp_device_request_resolve(
                    &d->base, LINQ_ERROR_TIMEOUT, "{\"error\":\"timeout\"}");
                zmtp_device_request_flush_w_check(&d->base);
            }
        }
    }
}

void
zmtp_device_retry_timeout_set(int val)
{
    retry_timeout = val;
}

int
zmtp_device_retry_timeout_get()
{
    return retry_timeout;
}

void
zmtp_device_max_retry_set(int val)
{
    max_retry = val;
}

int
zmtp_device_max_retry_get()
{
    return max_retry;
}

