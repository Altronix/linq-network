#include "linq_internal.h"
#include "node.h"
#include "nodes.h"
#include "request.h"

#define JSMN_HEADER
#include "jsmn/jsmn.h"

#define exe_on_error(linq, error, serial)                                      \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->err)                           \
            linq->callbacks->err(linq->context, error, "", serial);            \
    } while (0)
#define exe_on_heartbeat(linq, node_p)                                         \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->hb)                            \
            linq->callbacks->hb(linq->context, node_serial(*node_p), d);       \
    } while (0)
#define exe_on_alert(linq, node_p, a, e)                                       \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->alert)                         \
            linq->callbacks->alert(linq->context, a, e, node_p);               \
    } while (0)

// Main class
typedef struct linq_s
{
    void* context;
    zsock_t* sock;
    nodes_s* devices;
    nodes_s* nodes;
    linq_callbacks* callbacks;
} linq_s;

// A version on the wire is a byte
typedef int8_t version;

// Types of protocol messages
typedef enum
{
    TYPE_HEARTBEAT = FRAME_TYP_HEARTBEAT,
    TYPE_REQUEST = FRAME_TYP_REQUEST,
    TYPE_RESPONSE = FRAME_TYP_RESPONSE,
    TYPE_ALERT = FRAME_TYP_ALERT,
    TYPE_HELLO = FRAME_TYP_HELLO
} E_TYPE;

// parse a token from a json string inside a frame
static uint32_t
parse_token(zframe_t* f, jsmntok_t* t, char** ptr)
{
    uint32_t sz = t->end - t->start;
    *ptr = (char*)&zframe_data(f)[t->start];
    (*ptr)[sz] = 0;
    return sz;
}

// unholy footgun (parse many tokens)
static uint32_t
parse_tokens(
    zframe_t* f,
    uint32_t n_tokens,
    jsmntok_t** tokens_p,
    uint32_t n_tags,
    ...)
{
    char *tag = NULL, *cmp, **str;
    uint32_t taglen = 0;
    uint32_t count = 0;
    jsmntok_t* t = *tokens_p;
    for (uint32_t i = 0; i < n_tokens; i++) {
        if (t[i].type == JSMN_OBJECT || (t[i].type == JSMN_ARRAY)) continue;
        if (!tag) {
            taglen = parse_token(f, &t[i], &tag);
        } else {
            uint32_t c = n_tags << 1;
            va_list list;
            va_start(list, n_tags);
            while (c >= 2) {
                c -= 2;
                cmp = va_arg(list, char*);
                str = va_arg(list, char**);
                if (taglen == strlen(cmp) && !memcmp(tag, cmp, taglen)) {
                    parse_token(f, &t[i], str);
                    count++;
                }
            }
            va_end(list);
            tag = NULL;
        }
    }
    return count;
}

// read incoming zmq frame and test size is equal to expected
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

// read incoming zmq frame and test size is less than value
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

// read incoming zmq frame and test size is less than value
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

// read incoming zmq frame and test valid json
static zframe_t*
pop_alert(zmsg_t* msg, linq_alert_s* alert)
{
    memset(alert, 0, sizeof(linq_alert_s));
    int r, count;
    zframe_t* f = pop_le(msg, JSON_LEN);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, (char*)zframe_data(f), zframe_size(f), t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            f,
            r,
            &tokens,
            5,
            "who",   &alert->who,
            "what",  &alert->what,
            "siteId",&alert->where,
            "when",  &alert->when,
            "mesg",  &alert->mesg);
        // clang-format on
        if (!(count == 5)) zframe_destroy(&f);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

static zframe_t*
pop_email(zmsg_t* msg, linq_email_s* emails)
{
    memset(emails, 0, sizeof(linq_email_s));
    int r, count;
    zframe_t* f = pop_le(msg, JSON_LEN);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, (char*)zframe_data(f), zframe_size(f), t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            f,
            r,
            &tokens,
            5,
            "to0", &emails->to0,
            "to1", &emails->to1,
            "to2", &emails->to2,
            "to3", &emails->to3,
            "to4", &emails->to4);
        // clang-format on
        if (!(count == 5)) zframe_destroy(&f);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

// Write null terminated string into a frame data buffer
static int
print_null_terminated(char* c, uint32_t sz, zframe_t* f)
{
    uint32_t fsz = zframe_size(f);
    if (fsz + 1 <= sz) {
        memcpy(c, zframe_data(f), fsz);
        c[fsz] = 0;
        return 0;
    } else {
        return -1;
    }
}

// find a device in our device map and update the router id. insert device if hb
static node_s**
node_resolve(linq_s* l, nodes_s* devices, zframe_t** frames, bool hb)
{
    uint32_t rid_sz = zframe_size(frames[FRAME_RID_IDX]);
    uint8_t* rid = zframe_data(frames[FRAME_RID_IDX]);
    char sid[SID_LEN], pid[PID_LEN];
    print_null_terminated(sid, SID_LEN, frames[FRAME_SID_IDX]);
    print_null_terminated(pid, PID_LEN, frames[FRAME_HB_PID_IDX]);
    node_s** d = nodes_get(devices, sid);
    if (d) {
        node_heartbeat(*d);
        node_update_router(*d, rid, rid_sz);
    } else {
        if (hb && frames[FRAME_HB_PID_IDX]) {
            d = nodes_insert(l->devices, &l->sock, rid, rid_sz, sid, pid);
        }
    }
    return d;
}

// check the zmq request frames are valid and process the request
static E_LINQ_ERROR
process_request(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    ((void)l);
    ((void)msg);
    ((void)frames);
    return e;
    // TODO - if device exist, forward request to device and use callback to
    // forward response... else send 404
}

// check the zmq response frames are valid and process the response
static E_LINQ_ERROR
process_response(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *err, *dat;
    if ((zmsg_size(*msg) == 2) &&
        (err = frames[FRAME_RES_ERR_IDX] = pop_eq(*msg, 1)) &&
        (dat = frames[FRAME_RES_DAT_IDX] = pop_le(*msg, JSON_LEN))) {
        node_s** d = node_resolve(l, l->devices, frames, false);
        if (d) {
            node_recv(*d, zframe_data(err)[0], (const char*)zframe_data(dat));
            e = LINQ_ERROR_OK;
        }
    }
    return e;
}

// check the zmq alert frames are valid and process the alert
static E_LINQ_ERROR
process_alert(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    linq_alert_s alert;
    linq_email_s email;
    if (zmsg_size(*msg) == 3 &&
        (frames[FRAME_ALERT_PID_IDX] = pop_le(*msg, PID_LEN)) &&
        (frames[FRAME_ALERT_DAT_IDX] = pop_alert(*msg, &alert)) &&
        (frames[FRAME_ALERT_DST_IDX] = pop_email(*msg, &email))) {
        node_s** d = node_resolve(l, l->devices, frames, false);
        if (d) {
            exe_on_alert(l, d, &alert, &email);
            e = LINQ_ERROR_OK;
        }
    }

    return e;
}

// check the zmq hello message is valid and add a node if it does not exist
static E_LINQ_ERROR
process_hello(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    ((void)l);
    ((void)msg);
    ((void)frames);
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    node_s** n = node_resolve(l, l->nodes, frames, true);
    if (n) e = LINQ_ERROR_OK;
    return e;
}

// check the zmq heartbeat frames are valid and process the heartbeat
static E_LINQ_ERROR
process_heartbeat(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    if (zmsg_size(*msg) == 2 &&
        (frames[FRAME_HB_PID_IDX] = pop_le(*msg, PID_LEN)) &&
        (frames[FRAME_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        node_s** d = node_resolve(l, l->devices, frames, true);
        if (d) {
            exe_on_heartbeat(l, d);
            e = LINQ_ERROR_OK;
        }
    }
    return e;
}

// check the zmq header frames are valid and process the packet
static E_LINQ_ERROR
process_packet(linq_s* l, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    *msg = zmsg_recv(l->sock);
    if (*msg && zmsg_size(*msg) >= 4 &&
        (frames[FRAME_RID_IDX] = pop_le(*msg, RID_LEN)) &&
        (frames[FRAME_VER_IDX] = pop_eq(*msg, 1)) &&
        (frames[FRAME_TYP_IDX] = pop_eq(*msg, 1)) &&
        (frames[FRAME_SID_IDX] = pop_le(*msg, SID_LEN))) {
        switch ((E_TYPE)zframe_data(frames[FRAME_TYP_IDX])[0]) {
            case TYPE_HEARTBEAT: e = process_heartbeat(l, msg, frames); break;
            case TYPE_REQUEST: e = process_request(l, msg, frames); break;
            case TYPE_RESPONSE: e = process_response(l, msg, frames); break;
            case TYPE_ALERT: e = process_alert(l, msg, frames); break;
            case TYPE_HELLO: e = process_hello(l, msg, frames); break;
        }
    }
    return e;
}

// read zmq messages
static E_LINQ_ERROR
process_incoming(linq_s* l)
{
    int n = 0;
    zframe_t* frames[FRAME_MAX + 1];
    memset(frames, 0, sizeof(frames));
    zmsg_t* msg;
    E_LINQ_ERROR e = process_packet(l, &msg, frames);
    if (e) exe_on_error(l, e, "");
    zmsg_destroy(&msg);
    while (frames[n]) zframe_destroy(&frames[n++]);
    return e;
}

// Create main context for the caller
linq_s*
linq_create(linq_callbacks* cb, void* context)
{
    linq_s* l = linq_malloc(sizeof(linq_s));
    if (l) {
        memset(l, 0, sizeof(linq_s));
        l->devices = nodes_create();
        l->nodes = nodes_create();
        l->callbacks = cb;
        l->context = context;
    }
    return l;
}

// Free main context after use
void
linq_destroy(linq_s** linq_p)
{
    linq_s* l = *linq_p;
    *linq_p = NULL;
    nodes_destroy(&l->devices);
    nodes_destroy(&l->nodes);
    if (l->sock) zsock_destroy(&l->sock);
    linq_free(l);
}

// Listen for incoming device connections on "endpoint"
E_LINQ_ERROR
linq_listen(linq_s* l, const char* ep)
{
    if (l->sock) return LINQ_ERROR_BAD_ARGS;
    l->sock = zsock_new_router(ep);
    return l->sock ? LINQ_ERROR_OK : LINQ_ERROR_BAD_ARGS;
}

// poll network socket file handles
E_LINQ_ERROR
linq_poll(linq_s* l)
{
    int err;
    zmq_pollitem_t item = { zsock_resolve(l->sock), 0, ZMQ_POLLIN, 0 };
    err = zmq_poll(&item, 1, 5);
    if (err < 0) return err;
    if (item.revents && ZMQ_POLLIN) err = process_incoming(l);
    return err;
}

// get a device from the device map
node_s**
linq_device(linq_s* l, const char* serial)
{
    return nodes_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_node_count(linq_s* l)
{
    return nodes_size(l->devices);
}

// send a get request to a device connected to us
E_LINQ_ERROR
linq_node_send_get(
    linq_s* linq,
    const char* serial,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    node_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    node_send_get(*d, path, fn, context);
    return LINQ_ERROR_OK;
}

// send a post request to a device connected to us
E_LINQ_ERROR
linq_node_send_post(
    linq_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    node_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    node_send_post(*d, path, json, fn, context);
    return LINQ_ERROR_OK;
}

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_node_send_delete(
    linq_s* linq,
    const char* serial,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    node_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    node_send_delete(*d, path, fn, context);
    return LINQ_ERROR_OK;
}

// send a request to a device connected to us
E_LINQ_ERROR
linq_node_send(linq_s* linq, const char* serial, request_s* request)
{
    node_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    node_send(*d, &request);
    return LINQ_ERROR_OK;
}
