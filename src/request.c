#include "czmq.h"

#include "klib/klist.h"
#include "request.h"

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

request_s*
request_create(
    E_REQUEST_METHOD method,
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn on_complete,
    void* context)
{
    return request_create_mem(
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

request_s*
request_create_mem(
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

void
request_destroy(request_s** r_p)
{
    request_s* r = *r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_free(r);
}

void
request_sent_at(request_s* r, uint32_t at)
{
    r->sent_at = at;
}

void
request_router_id_set(request_s* r, uint8_t* rid, uint32_t rid_len)
{
    r->frames[FRAME_RID_IDX] = zframe_new(rid, rid_len);
    linq_assert(r->frames[FRAME_RID_IDX]);
}

const char*
request_serial_get(request_s* r)
{
    return (char*)zframe_data(r->frames[FRAME_SID_IDX]);
}

int
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

