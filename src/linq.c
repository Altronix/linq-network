#include "base64.h"
#include "containers.h"
#include "device.h"
#include "linq_internal.h"
#include "node.h"

#define JSMN_HEADER
#include "jsmn/jsmn.h"

#define exe_on_error(linq, error, serial)                                      \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->err)                           \
            linq->callbacks->err(linq->context, error, "", serial);            \
    } while (0)
#define exe_on_heartbeat(linq, device_p)                                       \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->hb)                            \
            linq->callbacks->hb(linq->context, device_serial(*device_p), d);   \
    } while (0)
#define exe_on_alert(linq, device_p, a, e)                                     \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->alert)                         \
            linq->callbacks->alert(linq->context, a, e, device_p);             \
    } while (0)

char g_frame_ver_0 = FRAME_VER_0;
char g_frame_typ_heartbeat = FRAME_TYP_HEARTBEAT;
char g_frame_typ_request = FRAME_TYP_REQUEST;
char g_frame_typ_response = FRAME_TYP_RESPONSE;
char g_frame_typ_alert = FRAME_TYP_ALERT;
char g_frame_typ_hello = FRAME_TYP_HELLO;
MAP_INIT(devices, device_s, device_destroy);
MAP_INIT(nodes, node_s, node_destroy);
LIST_INIT(sockets, zsock_t, zsock_destroy);
device_s** linq_device_from_frame(linq_s* l, zframe_t* frame);

// Main class
typedef struct linq_s
{
    void* context;
    list_sockets_s* routers;
    list_sockets_s* dealers;
    map_devices_s* devices;
    map_nodes_s* nodes;
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
parse_token(char* data, jsmntok_t* t, char** ptr)
{
    uint32_t sz = t->end - t->start;
    *ptr = &data[t->start];
    (*ptr)[sz] = 0;
    return sz;
}

// unholy footgun (parse many tokens)
static uint32_t
parse_tokens(
    char* data,
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
            taglen = parse_token(data, &t[i], &tag);
        } else {
            uint32_t c = n_tags << 1;
            va_list list;
            va_start(list, n_tags);
            while (c >= 2) {
                c -= 2;
                cmp = va_arg(list, char*);
                str = va_arg(list, char**);
                if (taglen == strlen(cmp) && !memcmp(tag, cmp, taglen)) {
                    parse_token(data, &t[i], str);
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
    int r, count, sz;
    zframe_t* f = pop_le(msg, JSON_LEN);
    sz = zframe_size(f);
    memcpy(alert->data, zframe_data(f), sz);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, alert->data, sz, t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            alert->data,
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
    int r, count, sz;
    zframe_t* f = pop_le(msg, JSON_LEN);
    sz = zframe_size(f);
    memcpy(emails->data, zframe_data(f), sz);
    jsmntok_t t[30], *tokens = t;
    jsmn_parser p;
    jsmn_init(&p);
    r = jsmn_parse(&p, emails->data, sz, t, 30);
    if (r >= 11) {
        // clang-format off
        count = parse_tokens(
            emails->data,
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

// A device is resolved by the serial number frame
static device_s**
device_resolve(
    zsock_t* sock,
    map_devices_s* map,
    zframe_t** frames,
    bool insert)
{
    uint32_t rid_sz = 0;
    uint8_t* rid = NULL;
    if (frames[FRAME_RID_IDX]) {
        rid_sz = zframe_size(frames[FRAME_RID_IDX]);
        rid = zframe_data(frames[FRAME_RID_IDX]);
    }
    char sid[SID_LEN], tid[TID_LEN] = { 0 };
    print_null_terminated(sid, SID_LEN, frames[FRAME_SID_IDX]);
    print_null_terminated(tid, TID_LEN, frames[FRAME_HB_TID_IDX]);
    device_s** d = map_devices_get(map, sid);
    if (d) {
        device_heartbeat(*d);
        if (rid) device_update_router(*d, rid, rid_sz);
    } else {
        if (insert) {
            device_s* node = device_create(sock, rid, rid_sz, sid, tid);
            if (node) d = map_devices_add(map, device_serial(node), &node);
        }
    }
    return d;
}

// Node is resolved by grabing object with base64_encoded key of the router id
static node_s**
node_resolve(zsock_t* sock, map_nodes_s* map, zframe_t** frames, bool insert)
{
    uint32_t rid_len = 0;
    uint8_t* rid = NULL;
    if (frames[FRAME_RID_IDX]) {
        rid_len = zframe_size(frames[FRAME_RID_IDX]);
        rid = zframe_data(frames[FRAME_RID_IDX]);
    }
    char sid[B64_RID_LEN];
    size_t sid_len = sizeof(sid);
    b64_encode((uchar*)sid, &sid_len, (uchar*)rid, rid_len);
    node_s** d = map_nodes_get(map, sid);
    if (d) {
        if (rid) node_update_router(*d, rid, rid_len);
    } else {
        if (insert) {
            node_s* node = node_create(sock, rid, rid_len, sid);
            if (node) d = map_nodes_add(map, node_serial(node), &node);
        }
    }
    return d;
}

// Broadcast x frames to each node (router frame is added per each node)
static void
foreach_node_forward_message(void* ctx, node_s** n)
{
    frames_s* frames = ctx;
    node_send_frames(*n, frames->n, frames->frames);
}

// A device has responded to a request from a node on linq->nodes. Forward
// the response to the node @ linq->nodes
static void
on_device_response(
    void* ctx,
    E_LINQ_ERROR error,
    const char* json,
    device_s** device)
{
    node_s** node = ctx;
    node_send_frames_n(
        *node,
        5,
        &g_frame_ver_0,                 // version
        1,                              //
        &g_frame_typ_response,          // type
        1,                              //
        device_serial(*device),         // serial
        strlen(device_serial(*device)), //
        error,                          // error
        1,                              //
        json,                           // data
        strlen(json)                    //
    );
}

// check the zmq request frames are valid and process the request
static E_LINQ_ERROR
process_request(linq_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *path = NULL, *data = NULL;
    if ((zmsg_size(*msg) >= 1) &&
        (path = frames[FRAME_REQ_PATH_IDX] = pop_le(*msg, 128))) {
        if (zmsg_size(*msg) == 1) {
            data = frames[FRAME_REQ_DATA_IDX] = pop_le(*msg, JSON_LEN);
        }
        node_s** n = node_resolve(sock, l->nodes, frames, false);
        device_s** d = linq_device_from_frame(l, frames[FRAME_SID_IDX]);
        if (n && d) {
            device_send(
                *d,
                (const char*)zframe_data(path),
                data ? (const char*)zframe_data(data) : NULL,
                on_device_response,
                n);
        } else {
            // TODO send 404 response (device not here)
        }
    }
    return e;
}

// check the zmq response frames are valid and process the response
static E_LINQ_ERROR
process_response(linq_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *err, *dat;
    if ((zmsg_size(*msg) == 2) &&
        (err = frames[FRAME_RES_ERR_IDX] = pop_eq(*msg, 1)) &&
        (dat = frames[FRAME_RES_DAT_IDX] = pop_le(*msg, JSON_LEN))) {
        e = LINQ_ERROR_OK;
        device_s** d = device_resolve(sock, l->devices, frames, false);
        if (d) {
            if (device_request_pending(*d)) {
                device_request_resolve(
                    *d, zframe_data(err)[0], (const char*)zframe_data(dat));
            }
        }
    }
    return e;
}

// check the zmq alert frames are valid and process the alert
static E_LINQ_ERROR
process_alert(linq_s* l, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    char alert_data[JSON_LEN];
    char email_data[JSON_LEN];
    linq_alert_s alert;
    linq_email_s email;
    memset(&alert, 0, sizeof(alert));
    memset(&email, 0, sizeof(email));
    alert.data = alert_data;
    email.data = email_data;
    if (zmsg_size(*msg) == 3 &&
        (frames[FRAME_ALERT_TID_IDX] = pop_le(*msg, TID_LEN)) &&
        (frames[FRAME_ALERT_DAT_IDX] = pop_alert(*msg, &alert)) &&
        (frames[FRAME_ALERT_DST_IDX] = pop_email(*msg, &email))) {
        device_s** d = device_resolve(socket, l->devices, frames, false);
        frames_s f = { 6, &frames[1] };
        if (d) {
            map_nodes_foreach(l->nodes, foreach_node_forward_message, &f);
            exe_on_alert(l, d, &alert, &email);
            e = LINQ_ERROR_OK;
        }
    }

    return e;
}

// check the zmq hello message is valid and add a node if it does not exist
static E_LINQ_ERROR
process_hello(linq_s* l, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
{
    ((void)l);
    ((void)msg);
    ((void)socket);
    ((void)frames);
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    node_s** s = node_resolve(socket, l->nodes, frames, true);
    if (s) e = LINQ_ERROR_OK;
    return e;
}

// check the zmq heartbeat frames are valid and process the heartbeat
static E_LINQ_ERROR
process_heartbeat(linq_s* l, zsock_t* s, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    if (zmsg_size(*msg) == 2 &&
        (frames[FRAME_HB_TID_IDX] = pop_le(*msg, TID_LEN)) &&
        (frames[FRAME_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        device_s** d = device_resolve(s, l->devices, frames, true);
        frames_s f = { 5, &frames[1] };
        if (d) {
            // TODO - this re broadcasting something that was broadcast to us
            // and causes infinite spamming loop. need new protocol item for
            // adding a device
            map_nodes_foreach(l->nodes, foreach_node_forward_message, &f);
            exe_on_heartbeat(l, d);
            e = LINQ_ERROR_OK;
        }
    }
    return e;
}

// check the zmq header frames are valid and process the packet
static E_LINQ_ERROR
process_packet(linq_s* l, zsock_t* s, bool router)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    int total_frames = 0;
    zframe_t* f[FRAME_MAX];
    memset(f, 0, sizeof(f));
    zmsg_t* msg = zmsg_recv(s);
    total_frames = zmsg_size(msg);

    f[FRAME_RID_IDX] = router ? (msg ? pop_le(msg, RID_LEN) : NULL) : NULL;

    if (msg && zmsg_size(msg) >= 3 && (!router || (router && f[0])) &&
        (f[FRAME_VER_IDX] = pop_eq(msg, 1)) &&
        (f[FRAME_TYP_IDX] = pop_eq(msg, 1)) &&
        (f[FRAME_SID_IDX] = pop_le(msg, SID_LEN))) {
        switch ((E_TYPE)zframe_data(f[FRAME_TYP_IDX])[0]) {
            case TYPE_HEARTBEAT: e = process_heartbeat(l, s, &msg, f); break;
            case TYPE_REQUEST: e = process_request(l, s, &msg, f); break;
            case TYPE_RESPONSE: e = process_response(l, s, &msg, f); break;
            case TYPE_ALERT: e = process_alert(l, s, &msg, f); break;
            case TYPE_HELLO: e = process_hello(l, s, &msg, f); break;
        }
    }
    if (e) exe_on_error(l, e, "");
    zmsg_destroy(&msg);
    for (int i = 0; i < total_frames; i++) {
        if (f[i]) zframe_destroy(&f[i]);
    }
    return e;
}

// Create main context for the caller
linq_s*
linq_create(linq_callbacks* cb, void* context)
{
    linq_s* l = linq_malloc(sizeof(linq_s));
    if (l) {
        memset(l, 0, sizeof(linq_s));
        l->devices = map_devices_create();
        l->nodes = map_nodes_create();
        l->routers = list_sockets_create();
        l->dealers = list_sockets_create();
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
    map_devices_destroy(&l->devices);
    map_nodes_destroy(&l->nodes);
    list_sockets_destroy(&l->routers);
    list_sockets_destroy(&l->dealers);
    // if (l->sock) zsock_destroy(&l->sock);
    linq_free(l);
}

// Listen for incoming device connections on "endpoint"
E_LINQ_ERROR
linq_listen(linq_s* l, const char* ep)
{
    zsock_t* socket = zsock_new_router(ep);
    if (socket) {
        list_sockets_push(l->routers, &socket);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

// connect to a remote linq and send hello frames
E_LINQ_ERROR
linq_connect(linq_s* l, const char* ep)
{
    E_LINQ_ERROR e = LINQ_ERROR_BAD_ARGS;
    zsock_t* socket = zsock_new_dealer(ep);
    if (socket) {
        list_sockets_push(l->dealers, &socket);
        node_s* n = node_create(list_sockets_front(l->dealers), NULL, 0, NULL);
        if (n) {
            node_send_hello(n);
            map_nodes_add(l->nodes, ep, &n);
            return LINQ_ERROR_OK;
        }
    }
    return e;
}

// loop through each node and resolve any requests that have timed out
static void
foreach_node_check_request_timeout(void* ctx, device_s** n)
{
    ((void)ctx);
    if (device_request_pending(*n) &&
        device_request_sent_at(*n) + 10000 <= sys_tick()) {
        device_request_resolve(
            *n, LINQ_ERROR_TIMEOUT, "{\"error\":\"timeout\"}");
    }
}

static E_LINQ_ERROR
poll_sockets(linq_s* l, list_sockets_s* ss, bool is_router)
{
    int err, n = 0;
    zmq_pollitem_t items[MAX_CONNECTIONS];
    memset(items, 0, sizeof(items));

    for (sockets_item_s* s = ss->head; s != ss->tail; s = s->next) {
        items[n].socket = zsock_resolve(s->data);
        items[n].events = ZMQ_POLLIN;
        n++;
    }
    n = 0;

    // Process sockets
    err = zmq_poll(items, list_sockets_size(ss), 5);
    if (!(err < 0)) {
        for (sockets_item_s* s = ss->head; s != ss->tail; s = s->next) {
            if (items[n++].revents && ZMQ_POLLIN) {
                err = process_packet(l, s->data, is_router);
            }
        }
    }

    return err;
}

// poll network socket file handles
E_LINQ_ERROR
linq_poll(linq_s* l)
{
    int e;

    // Poll sockets
    e = poll_sockets(l, l->routers, true) | poll_sockets(l, l->dealers, false);

    // Loop through devices
    map_devices_foreach(l->devices, foreach_node_check_request_timeout, l);

    return e;
}

device_s**
linq_device_from_frame(linq_s* l, zframe_t* frame)
{
    char sid[SID_LEN];
    print_null_terminated(sid, SID_LEN, frame);
    return linq_device(l, sid);
}

// get a device from the device map
device_s**
linq_device(linq_s* l, const char* serial)
{
    return map_devices_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_device_count(linq_s* l)
{
    return map_devices_size(l->devices);
}

// return how many nodes are connected to linq
uint32_t
linq_nodes_count(linq_s* l)
{
    return map_nodes_size(l->nodes);
}

// send a get request to a device connected to us
E_LINQ_ERROR
linq_device_send_get(
    linq_s* linq,
    const char* serial,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    device_send_get(*d, path, fn, context);
    return LINQ_ERROR_OK;
}

// send a post request to a device connected to us
E_LINQ_ERROR
linq_device_send_post(
    linq_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    device_send_post(*d, path, json, fn, context);
    return LINQ_ERROR_OK;
}

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_device_send_delete(
    linq_s* linq,
    const char* serial,
    const char* path,
    linq_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_device(linq, serial);
    if (!d) return LINQ_ERROR_DEVICE_NOT_FOUND;
    device_send_delete(*d, path, fn, context);
    return LINQ_ERROR_OK;
}
