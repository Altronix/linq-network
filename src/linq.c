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

// Packet containing incoming "frames" from net
typedef struct
{
    router router;
    char serial[64];
    version version;
    type type;
    union
    {
        struct heartbeat
        {
            char product[64];
            char site_id[64];
        } heartbeat;

        struct alert
        {
            void* product;
            linq_alert* alert;
        } alert;

        struct response
        {
            int error;
            void* data;
        } response;
    };
} packet;

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
device_resolve(device_map* devices, const char* serial, router* router)
{
    device** d = device_map_get(devices, serial);
    if (d) {
        device_heartbeat(*d);
        device_update_router(*d, router);
    }
    return d;
}

static e_linq_error
pop_incoming(packet* p, zmsg_t* m)
{
    zframe_t *rid, *ver, *typ, *sid;
    e_linq_error e = e_linq_protocol;
    if (zmsg_size(m) >= 4) {
        rid = zmsg_pop(m);
        ver = zmsg_pop(m);
        typ = zmsg_pop(m);
        sid = zmsg_pop(m);
        if (((p->router.sz = zframe_size(rid)) <= RID_LEN) &&
            (zframe_size(ver) == 1) && (zframe_size(typ) == 1) &&
            (zframe_size(sid) <= SID_LEN)) {
            memcpy(p->router.id, zframe_data(rid), zframe_size(rid));
            snprintf(p->serial, SID_LEN, "%s", zframe_data(sid));
            p->version = zframe_data(ver)[0];
            p->type = zframe_data(typ)[0];
            e = e_linq_ok;
        }
        zframe_destroy(&sid);
        zframe_destroy(&rid);
        zframe_destroy(&ver);
        zframe_destroy(&typ);
    }
    return e;
}

static e_linq_error
pop_heartbeat(packet* p, zmsg_t* m)
{
    zframe_t *pid, *sid;
    e_linq_error e = e_linq_protocol;
    if (zmsg_size(m) == 2) {
        pid = zmsg_pop(m);
        sid = zmsg_pop(m);
        if ((zframe_size(pid) <= PID_LEN) && (zframe_size(sid) <= SITE_LEN)) {
            snprintf(p->heartbeat.product, PID_LEN, "%s", zframe_data(pid));
            snprintf(p->heartbeat.site_id, SITE_LEN, "%s", zframe_data(sid));
            e = e_linq_ok;
        }
        zframe_destroy(&pid);
        zframe_destroy(&sid);
    }
    return e;
}

static e_linq_error
pop_response(packet* p, zmsg_t* m)
{
    zframe_t *err, *dat;
    e_linq_error e = e_linq_protocol;
    if (zmsg_size(m) == 2) {
        err = zmsg_pop(m);
        dat = zmsg_pop(m);
        // TODO
        ((void)p);
        zframe_destroy(&err);
        zframe_destroy(&dat);
    }
    return e;
}

static e_linq_error
pop_alert(packet* p, zmsg_t* m)
{
    e_linq_error e = e_linq_protocol;
    zframe_t *product, *alert, *mail;
    if (zmsg_size(m) == 3) {
        product = zmsg_pop(m);
        alert = zmsg_pop(m);
        mail = zmsg_pop(m);
        // TODO
        ((void)p);
        zframe_destroy(&product);
        zframe_destroy(&alert);
        zframe_destroy(&mail);
    }
    return e;
}

// Process an assumed heartbeat
static e_linq_error
process_heartbeat(linq* l, packet* hb)
{
    device** d = device_resolve(l->devices, hb->serial, &hb->router);
    if (!d) {
        d = device_map_insert(
            l->devices,
            &l->sock,
            &hb->router,
            hb->serial,
            hb->heartbeat.product);
    }
    on_heartbeat(l, d);
    return e_linq_ok;
}

// Process an assumed response
static e_linq_error
process_response(linq* l, packet* response)
{
    ((void)l);
    ((void)response);
    return -1;
}

// Process an assumed alert
static e_linq_error
process_alert(linq* l, packet* alert)
{
    ((void)l);
    ((void)alert);
    return -1;
}

// Start reading data from the network
static e_linq_error
process_incoming(linq* l)
{
    packet p;
    e_linq_error e = e_linq_protocol;
    zmsg_t* msg = zmsg_recv(l->sock);
    if (msg && ((e = pop_incoming(&p, msg)) == e_linq_ok)) {
        switch (p.type) {
            case heartbeat:
                if ((e = pop_heartbeat(&p, msg)) == e_linq_ok) {
                    process_heartbeat(l, &p);
                }
                break;
            case request: break;
            case response:
                if ((e = pop_response(&p, msg)) == e_linq_ok)
                    process_response(l, &p);
                break;
            case alert:
                if ((e = pop_alert(&p, msg)) == e_linq_ok) {
                    process_alert(l, &p);
                }
                break;
        }
    }
    if (e) on_error(l, e, "");
    zmsg_destroy(&msg);
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
