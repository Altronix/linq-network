#include "czmq.h"

#include "containers.h"
#include "device_request.h"

static zframe_t*
write_path_to_frame(const char* method, const char* path, uint32_t path_len)
{
    zframe_t* frame;
    bool has_prefix = true;
    uint32_t sz = strlen(method) + path_len + (has_prefix ? 0 : 1) + 2;
    if (!(*path == '/')) {
        has_prefix = false;
        sz++;
    }
    frame = zframe_new(NULL, sz);
    if (frame) {
        snprintf(
            (char*)zframe_data(frame),
            sz,
            "%s %s%s",
            method,
            has_prefix ? "" : "/",
            path);
    }
    return frame;
}

static zframe_t*
path_to_frame(E_REQUEST_METHOD method, const char* path, uint32_t path_len)
{
    zframe_t* frame = NULL;

    switch (method) {
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

device_request_s*
device_request_create(
    E_REQUEST_METHOD method,
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn on_complete,
    void* context)
{
    return device_request_create_mem(
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

device_request_s*
device_request_create_mem(
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
    device_request_s* r = linq_malloc(sizeof(device_request_s));
    if (r) {
        memset(r, 0, sizeof(device_request_s));
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
            device_request_destroy(&r);
        }
    }
    return r;
}

device_request_s*
device_request_create_from_frames(
    zframe_t* serial,
    zframe_t* path,
    zframe_t* data,
    linq_request_complete_fn fn,
    void* ctx)
{
    device_request_s* r = linq_malloc(sizeof(device_request_s));
    if (r) {
        memset(r, 0, sizeof(device_request_s));
        r->on_complete = fn;
        r->ctx = ctx;
        r->frames[FRAME_VER_IDX] = zframe_new("\0", 1);
        r->frames[FRAME_TYP_IDX] = zframe_new("\1", 1);
        r->frames[FRAME_SID_IDX] = zframe_dup(serial);
        r->frames[FRAME_REQ_PATH_IDX] = zframe_dup(path);
        if (data) r->frames[FRAME_REQ_DATA_IDX] = zframe_dup(data);
        if (!(r->frames[FRAME_VER_IDX] && r->frames[FRAME_TYP_IDX] &&
              r->frames[FRAME_SID_IDX] && r->frames[FRAME_REQ_PATH_IDX] &&
              ((data && r->frames[FRAME_REQ_DATA_IDX]) || !data))) {
            device_request_destroy(&r);
        }
    }
    return r;
}

void
device_request_destroy(device_request_s** r_p)
{
    device_request_s* r = *r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_free(r);
}

uint32_t
device_request_sent_at(device_request_s* r)
{
    return r->sent_at;
}

void
device_request_router_id_set(
    device_request_s* r,
    uint8_t* rid,
    uint32_t rid_len)
{
    r->frames[FRAME_RID_IDX] = zframe_new(rid, rid_len);
    linq_assert(r->frames[FRAME_RID_IDX]);
}

const char*
device_request_serial_get(device_request_s* r)
{
    return (char*)zframe_data(r->frames[FRAME_SID_IDX]);
}

int
device_request_send(device_request_s* r, zsock_t* sock)
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

