#include "device.h"
#include "device_map.h"
#include "linq_internal.h"

#define JSMN_HEADER
#include "jsmn/jsmn.h"

// Main class
typedef struct linq
{
    void* context;
    zsock_t* sock;
    device_map* devices;
    linq_callbacks* callbacks;
} linq;

// A version on the wire is a byte
typedef int8_t version;

// Types of protocol messages
typedef enum
{
    heartbeat = 0,
    request = 1,
    response = 2,
    alert = 3
} type;

// read incoming zmq frame and assert size is equal to expected
static zframe_t*
pop_eq(zmsg_t* msg, uint32_t expect)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) == expect) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

// read incoming zmq frame and assert size is less than value
static zframe_t*
pop_lt(zmsg_t* msg, uint32_t lt)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) < lt) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

// read incoming zmq frame and assert size is less than value
static zframe_t*
pop_le(zmsg_t* msg, uint32_t le)
{
    zframe_t* frame = zmsg_pop(msg);
    if (zframe_size(frame) <= le) {
        return frame;
    } else {
        zframe_destroy(&frame);
        return NULL;
    }
}

static void
on_error(linq* l, e_linq_error e, const char* serial)
{
    if (l->callbacks && l->callbacks->err) {
        l->callbacks->err(l->context, e, "", serial);
    }
}

static void
on_heartbeat(linq* l, device** d)
{
    if (l->callbacks && l->callbacks->hb) {
        l->callbacks->hb(l->context, device_serial(*d), d);
    }
}

static device**
device_resolve(
    device_map* devices,
    const char* serial,
    uint8_t* router_id,
    uint32_t router_id_sz)
{
    device** d = device_map_get(devices, serial);
    if (d) {
        device_heartbeat(*d);
        device_update_router(*d, router_id, router_id_sz);
    }
    return d;
}

static e_linq_error
process_request(linq* l, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    ((void)l);
    ((void)frames);
    return e;
}

static e_linq_error
process_response(linq* l, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    ((void)l);
    ((void)frames);
    return e;
}
static e_linq_error
process_alert(linq* l, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    ((void)l);
    ((void)frames);
    return e;
}

// Process an assumed heartbeat
static e_linq_error
process_heartbeat(linq* l, zmsg_t** msg, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    uint32_t rid_sz = zframe_size(frames[PACKET_RID_IDX]);
    uint8_t* rid = zframe_data(frames[PACKET_RID_IDX]);
    char* sid = (char*)zframe_data(frames[PACKET_SID_IDX]);
    if (zmsg_size(*msg) == 2 &&
        (frames[PACKET_HB_PID_IDX] = pop_le(*msg, PID_LEN)) &&
        (frames[PACKET_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        device** d = device_resolve(l->devices, sid, rid, rid_sz);
        if (!d) {
            d = device_map_insert(
                l->devices,
                &l->sock,
                rid,
                rid_sz,
                sid,
                (char*)zframe_data(frames[PACKET_HB_PID_IDX]));
        }
        on_heartbeat(l, d);
        e = e_linq_ok;
    }
    return e;
}

static e_linq_error
process_packet(linq* l, zmsg_t** msg, zframe_t** frames)
{
    e_linq_error e = e_linq_protocol;
    *msg = zmsg_recv(l->sock);
    if (*msg && zmsg_size(*msg) >= 4 &&
        (frames[PACKET_RID_IDX] = pop_le(*msg, RID_LEN)) &&
        (frames[PACKET_VER_IDX] = pop_eq(*msg, 1)) &&
        (frames[PACKET_TYP_IDX] = pop_eq(*msg, 1)) &&
        (frames[PACKET_SID_IDX] = pop_le(*msg, SID_LEN))) {
        switch ((type)zframe_data(frames[PACKET_TYP_IDX])[0]) {
            case heartbeat: e = process_heartbeat(l, msg, frames); break;
            case request: break;
            case response: break;
            case alert: break;
        }
    }
    return e;
}

static e_linq_error
process_incoming(linq* l)
{
    int n = 0;
    zframe_t* frames[PACKET_MAX];
    memset(frames, 0, sizeof(frames));
    zmsg_t* msg;
    e_linq_error e = process_packet(l, &msg, frames);
    if (e) on_error(l, e, "");
    zmsg_destroy(&msg);
    while (frames[n]) zframe_destroy(&frames[n++]);
    return e;
}

// Create main context for the caller
linq*
linq_create(linq_callbacks* cb, void* context)
{
    linq* l = linq_malloc(sizeof(linq));
    if (l) {
        memset(l, 0, sizeof(linq));
        l->devices = device_map_create();
        l->callbacks = cb;
        l->context = context;
    }
    return l;
}

// Free main context after use
void
linq_destroy(linq** linq_p)
{
    linq* l = *linq_p;
    *linq_p = NULL;
    device_map_destroy(&l->devices);
    linq_free(l);
}

// Listen for incoming device connections on "endpoint"
e_linq_error
linq_listen(linq* l, const char* ep)
{
    if (l->sock) return e_linq_bad_args;
    l->sock = zsock_new_router(ep);
    return l->sock ? e_linq_ok : e_linq_bad_args;
}

// poll network socket file handles
e_linq_error
linq_poll(linq* l)
{
    int err;
    zmq_pollitem_t item = { zsock_resolve(l->sock), 0, ZMQ_POLLIN, 0 };
    err = zmq_poll(&item, 1, 1000);
    if (err < 0) return err;
    if (item.revents && ZMQ_POLLIN) { err = process_incoming(l); }
    return err;
}
device**
linq_device(linq* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

uint32_t
linq_device_count(linq* l)
{
    return device_map_size(l->devices);
}
