#include "linq_internal.h"

// Main class
typedef struct linq
{
    void* context;
    zsock_t* sock;
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
    zframe_t* router;
    version version;
    type type;
    zframe_t* serial;
    union
    {
        struct heartbeat
        {
            zframe_t* product;
            zframe_t* site_id;
        } heartbeat;

        struct alert
        {
            zframe_t* product;
            zframe_t* alert;
            zframe_t* mail;
        } alert;

        struct response
        {
            zframe_t* error;
            zframe_t* data;
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

// Read a valid version frame
static version
pop_version(zmsg_t* msg)
{
    zframe_t* f = zmsg_pop(msg);
    version v = zframe_size(f) == 1 ? zframe_data(f)[0] : -1;
    zframe_destroy(&f);
    return v;
}

// Read a valid type frame
static type
pop_type(zmsg_t* msg)
{
    zframe_t* f = zmsg_pop(msg);
    type t = zframe_size(f) == 1 ? zframe_data(f)[0] : -1;
    zframe_destroy(&f);
    return t;
}

// Read and parse JSON
static void
pop_json(zmsg_t* msg)
{
    ((void)msg);
    // TODO - add json lib
}

// Process an assumed heartbeat
static e_linq_error
process_heartbeat(linq* l, packet* hb)
{
    ((void)l);
    ((void)hb);
    return -1;
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
process_alert(linq* l, zmsg_t* msg)
{
    ((void)l);
    ((void)msg);
    return -1;
}

// Finish reading data from network
static e_linq_error
process_incoming_packet(linq* l, zmsg_t* msg, packet* p)
{
    e_linq_error e = e_linq_protocol;
    switch (p->type) {
        case heartbeat:
            if ((zmsg_size(msg) == 2)) {
                p->heartbeat.product = zmsg_pop(msg);
                p->heartbeat.site_id = zmsg_pop(msg);
                e = process_heartbeat(l, p);
                zframe_destroy(&p->heartbeat.product);
                zframe_destroy(&p->heartbeat.site_id);
            }
            break;
        case request:
            if ((zmsg_size(msg) == 2)) {}
            break;
        case response:
            if ((zmsg_size(msg) == 2)) {
                p->response.error = zmsg_pop(msg);
                p->response.data = zmsg_pop(msg);
                e = process_response(l, p);
                zframe_destroy(&p->response.error);
                zframe_destroy(&p->response.data);
            }
            break;
        case alert:
            if ((zmsg_size(msg) == 3)) {
                p->alert.product = zmsg_pop(msg);
                p->alert.alert = zmsg_pop(msg);
                p->alert.mail = zmsg_pop(msg);
                zframe_destroy(&p->alert.product);
                zframe_destroy(&p->alert.alert);
                zframe_destroy(&p->alert.mail);
            }
            break;
    }
    return e;
}

// Read legacy data from the network
static e_linq_error
process_incoming_packet_legacy(linq* l, zmsg_t* msg, packet* pack)
{
    ((void)l);
    ((void)msg);
    ((void)pack);
    return e_linq_protocol;
}

// Start reading data from the network
static e_linq_error
process_incoming(linq* l)
{
    e_linq_error e = e_linq_protocol;
    zmsg_t* msg = zmsg_recv(l->sock);
    if ((msg && zmsg_size(msg) >= 4)) {
        packet p;
        p.router = zmsg_pop(msg);
        p.version = pop_version(msg);
        p.type = pop_type(msg);
        p.serial = zmsg_pop(msg);
        e = p.version == 0 ? process_incoming_packet(l, msg, &p)
                           : process_incoming_packet_legacy(l, msg, &p);
        zframe_destroy(&p.router);
        zframe_destroy(&p.serial);
    }
    if (e) on_error(l, e_linq_protocol, "");
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
