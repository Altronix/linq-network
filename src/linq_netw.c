// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base64.h"
#include "containers.h"
#include "device.h"
#include "linq_netw_internal.h"
#include "node.h"
#include "sys.h"

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
#define exe_on_ctrlc(linq)                                                     \
    do {                                                                       \
        if (linq->callbacks && linq->callbacks->ctrlc)                         \
            linq->callbacks->ctrlc(linq->context);                             \
    } while (0)

char g_frame_ver_0 = FRAME_VER_0;
char g_frame_typ_heartbeat = FRAME_TYP_HEARTBEAT;
char g_frame_typ_request = FRAME_TYP_REQUEST;
char g_frame_typ_response = FRAME_TYP_RESPONSE;
char g_frame_typ_alert = FRAME_TYP_ALERT;
char g_frame_typ_hello = FRAME_TYP_HELLO;
MAP_INIT(device, device_s, device_destroy);
MAP_INIT(node, node_s, node_destroy);
MAP_INIT(socket, zsock_t, zsock_destroy);
device_s** linq_netw_device_from_frame(linq_netw_s* l, zframe_t* frame);

// Main class
typedef struct linq_netw_s
{
    void* context;
    socket_map_s* routers;
    socket_map_s* dealers;
    device_map_s* devices;
    node_map_s* nodes;
    bool shutdown;
    const linq_netw_callbacks* callbacks;
} linq_netw_s;

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
        if (t[i].type == JSMN_OBJECT || (t[i].type == JSMN_ARRAY)) {
            tag = NULL;
            continue;
        }
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
pop_alert(zmsg_t* msg, linq_netw_alert_s* alert)
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
pop_email(zmsg_t* msg, linq_netw_email_s* emails)
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
device_resolve(zsock_t* sock, device_map_s* map, zframe_t** frames, bool insert)
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
    device_s** d = device_map_get(map, sid);
    if (d) {
        device_heartbeat(*d);
        if (rid) device_update_router(*d, rid, rid_sz);
    } else {
        if (insert) {
            device_s* node = device_create(sock, rid, rid_sz, sid, tid);
            if (node) d = device_map_add(map, device_serial(node), &node);
        }
    }
    return d;
}

// Node is resolved by grabing object with base64_encoded key of the router id
static node_s**
node_resolve(zsock_t* sock, node_map_s* map, zframe_t** frames, bool insert)
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
    node_s** d = node_map_get(map, sid);
    if (d) {
        if (rid) node_update_router(*d, rid, rid_len);
    } else {
        if (insert) {
            node_s* node = node_create(sock, rid, rid_len, sid);
            if (node) d = node_map_add(map, node_serial(node), &node);
        }
    }
    return d;
}

// Broadcast x frames to each node (router frame is added per each node)
static void
foreach_node_forward_message(node_map_s* self, void* ctx, node_s** n)
{
    ((void)self);
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
    int16_t e = error;
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
        &e,                             // error
        2,                              //
        json,                           // data
        strlen(json)                    //
    );
}

// check the zmq request frames are valid and process the request
static E_LINQ_ERROR
process_request(linq_netw_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *path = NULL, *data = NULL;
    char url[128] = { 0 }, json[JSON_LEN] = { 0 };
    if ((zmsg_size(*msg) >= 1) &&
        (path = frames[FRAME_REQ_PATH_IDX] = pop_le(*msg, 128))) {
        if (zmsg_size(*msg) == 1) {
            data = frames[FRAME_REQ_DATA_IDX] = pop_le(*msg, JSON_LEN);
        }
        node_s** n = node_resolve(sock, l->nodes, frames, false);
        device_s** d = linq_netw_device_from_frame(l, frames[FRAME_SID_IDX]);
        if (n && d) {
            print_null_terminated(url, sizeof(url), path);
            if (data) print_null_terminated(json, sizeof(json), data);
            device_send(*d, url, data ? json : NULL, on_device_response, n);
            e = LINQ_ERROR_OK;
        } else {
            // TODO send 404 response (device not here)
        }
    }
    return e;
}

// check the zmq response frames are valid and process the response
static E_LINQ_ERROR
process_response(linq_netw_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *err, *dat;
    int16_t err_code;
    char json[JSON_LEN] = { 0 };
    if ((zmsg_size(*msg) == 2) &&
        (err = frames[FRAME_RES_ERR_IDX] = pop_eq(*msg, 2)) &&
        (dat = frames[FRAME_RES_DAT_IDX] = pop_le(*msg, JSON_LEN))) {
        e = LINQ_ERROR_OK;
        device_s** d = device_resolve(sock, l->devices, frames, false);
        if (d) {
            print_null_terminated(json, sizeof(json), dat);
            if (device_request_pending(*d)) {
                err_code = zframe_data(err)[1] | zframe_data(err)[0] << 8;
                if (err_code == LINQ_ERROR_504) {
                    if (device_request_retry_count(*d) >= LINQ_NETW_MAX_RETRY) {
                        device_request_resolve(*d, err_code, json);
                        device_request_flush_w_check(*d);
                    } else {
                        uint32_t retry = sys_tick() + LINQ_NETW_RETRY_TIMEOUT;
                        device_request_retry_at_set(*d, retry);
                    }
                } else {
                    device_request_resolve(*d, err_code, json);
                    device_request_flush_w_check(*d);
                }
            }
        }
    }
    return e;
}

// check the zmq alert frames are valid and process the alert
static E_LINQ_ERROR
process_alert(linq_netw_s* l, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    char alert_data[JSON_LEN];
    char email_data[JSON_LEN];
    linq_netw_alert_s alert;
    linq_netw_email_s email;
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
            if (device_no_hops(*d)) {
                // We only broadcast when the device is directly connected
                // otherwize, nodes would rebroadcast to eachother infinite
                node_map_foreach(l->nodes, foreach_node_forward_message, &f);
            }
            exe_on_alert(l, d, &alert, &email);
            e = LINQ_ERROR_OK;
        }
    }

    return e;
}

// check the zmq hello message is valid and add a node if it does not exist
static E_LINQ_ERROR
process_hello(linq_netw_s* l, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
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
process_heartbeat(linq_netw_s* l, zsock_t* s, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    if (zmsg_size(*msg) == 2 &&
        (frames[FRAME_HB_TID_IDX] = pop_le(*msg, TID_LEN)) &&
        (frames[FRAME_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        device_s** d = device_resolve(s, l->devices, frames, true);
        frames_s f = { 5, &frames[1] };
        if (d) {
            if (device_no_hops(*d)) {
                // We only broadcast when the device is directly connected
                // otherwize, nodes would rebroadcast to eachother infinite
                node_map_foreach(l->nodes, foreach_node_forward_message, &f);
            }
            exe_on_heartbeat(l, d);
            e = LINQ_ERROR_OK;
        }
    }
    return e;
}

// check the zmq header frames are valid and process the packet
static E_LINQ_ERROR
process_packet(linq_netw_s* l, zsock_t* s, bool router)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    int total_frames = 0, start = 1;
    zframe_t* f[FRAME_MAX];
    memset(f, 0, sizeof(f));
    zmsg_t* msg = zmsg_recv(s); // TODO can be null
    total_frames = zmsg_size(msg);
    if (total_frames > FRAME_MAX) total_frames = FRAME_MAX;

    if (router) {
        f[FRAME_RID_IDX] = pop_le(msg, RID_LEN);
        start = FRAME_RID_IDX;
    }

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
        if (f[start]) zframe_destroy(&f[start]);
        start++;
    }
    return e;
}

// Create main context for the caller
linq_netw_s*
linq_netw_create(const linq_netw_callbacks* cb, void* context)
{
    linq_netw_s* l = linq_netw_malloc(sizeof(linq_netw_s));
    if (l) linq_netw_init(l, cb, context);
    return l;
}

// Free main context after use
void
linq_netw_destroy(linq_netw_s** linq_netw_p)
{
    linq_netw_s* l = *linq_netw_p;
    *linq_netw_p = NULL;
    linq_netw_deinit(l);
    linq_netw_free(l);
}

void
linq_netw_init(linq_netw_s* l, const linq_netw_callbacks* cb, void* context)
{
    memset(l, 0, sizeof(linq_netw_s));
    l->devices = device_map_create();
    l->nodes = node_map_create();
    l->routers = socket_map_create();
    l->dealers = socket_map_create();
    l->callbacks = cb;
    l->context = context ? context : l;
}

void
linq_netw_deinit(linq_netw_s* l)
{
    device_map_destroy(&l->devices);
    node_map_destroy(&l->nodes);
    socket_map_destroy(&l->routers);
    socket_map_destroy(&l->dealers);
}

void
linq_netw_context_set(linq_netw_s* linq, void* ctx)
{
    linq->context = ctx;
}

// Listen for incoming device connections on "endpoint"
linq_netw_socket
linq_netw_listen(linq_netw_s* l, const char* ep)
{
    zsock_t* socket = zsock_new_router(ep);
    if (socket) {
        socket_map_add(l->routers, ep, &socket);
        return socket_map_key(l->routers, ep);
    } else {
        return LINQ_ERROR_SOCKET;
    }
}

// connect to a remote linq and send hello frames
linq_netw_socket
linq_netw_connect(linq_netw_s* l, const char* ep)
{
    zsock_t* socket = zsock_new_dealer(ep);
    if (socket) {
        socket_map_add(l->dealers, ep, &socket);
        node_s* n = node_create(*socket_map_get(l->dealers, ep), NULL, 0, ep);
        if (n) {
            node_send_hello(n);
            node_map_add(l->nodes, ep, &n);
        }
        return socket_map_key(l->dealers, ep);
    }
    return LINQ_ERROR_SOCKET;
}

static void
foreach_device_remove_if_sock_eq(
    device_map_s* self,
    void* ctx,
    device_s** device_p)
{
    zsock_t* eq = ctx;
    linq_netw_socket_s* socket = ((linq_netw_socket_s*)*device_p);
    if (eq == socket->sock) device_map_remove(self, device_serial(*device_p));
}

static void
foreach_node_remove_if_sock_eq(node_map_s* self, void* ctx, node_s** device_p)
{
    zsock_t* eq = ctx;
    linq_netw_socket_s* socket = ((linq_netw_socket_s*)*device_p);
    if (eq == socket->sock) node_map_remove(self, node_serial(*device_p));
}

static void
remove_devices(zsock_t** s, device_map_s* devices)
{
    device_map_foreach(devices, foreach_device_remove_if_sock_eq, *s);
}

static void
remove_nodes(zsock_t** s, node_map_s* nodes)
{
    node_map_foreach(nodes, foreach_node_remove_if_sock_eq, *s);
}

E_LINQ_ERROR
linq_netw_close_router(linq_netw_s* l, linq_netw_socket handle)
{
    zsock_t** s = socket_map_resolve(l->routers, handle);
    if (s) {
        remove_devices(s, l->devices);
        socket_map_remove_iter(l->routers, handle);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

E_LINQ_ERROR
linq_netw_close_dealer(linq_netw_s* l, linq_netw_socket handle)
{
    zsock_t** s = socket_map_resolve(l->dealers, handle);
    if (s) {
        remove_devices(s, l->devices);
        remove_nodes(s, l->nodes);
        socket_map_remove_iter(l->dealers, handle);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

// loop through each node and resolve any requests that have timed out
static void
foreach_device_check_request(device_map_s* self, void* ctx, device_s** d)
{
    ((void)self);
    ((void)ctx);
    if (device_request_pending(*d)) {
        uint32_t tick = sys_tick();
        uint32_t retry_at = device_request_retry_at(*d);
        if (device_request_sent_at(*d) + 10000 <= tick) {
            device_request_resolve(
                *d, LINQ_ERROR_TIMEOUT, "{\"error\":\"timeout\"}");
            device_request_flush_w_check(*d);
        } else if (retry_at && retry_at <= tick) {
            device_request_retry(*d);
        }
    }
}

typedef struct foreach_socket_process_context
{
    linq_netw_s* network;
    zmq_pollitem_t** poll_p;
    bool is_router;
} foreach_socket_process_context;

static void
foreach_socket_process(socket_map_s* self, void* context, zsock_t** sock_p)
{
    ((void)self);
    foreach_socket_process_context* ctx = context;
    if ((*(zmq_pollitem_t**)ctx->poll_p)->revents && ZMQ_POLLIN) {
        int err = process_packet(ctx->network, *sock_p, ctx->is_router);
        ((void)err);
    }
    (*(zmq_pollitem_t**)ctx->poll_p)++;
}

static void
foreach_socket_populate_poll(socket_map_s* self, void* ctx, zsock_t** sock_p)
{
    ((void)self);
    (*(zmq_pollitem_t**)ctx)->socket = zsock_resolve(*sock_p);
    (*(zmq_pollitem_t**)ctx)->events = ZMQ_POLLIN;
    (*(zmq_pollitem_t**)ctx)++;
}

static void
populate_sockets(socket_map_s* h, zmq_pollitem_t** ptr_p)
{
    socket_map_foreach(h, foreach_socket_populate_poll, ptr_p);
}

// poll network socket file handles
E_LINQ_ERROR
linq_netw_poll(linq_netw_s* l, int32_t ms)
{
    zmq_pollitem_t items[MAX_CONNECTIONS], *ptr = items;
    int err, n_router = socket_map_size(l->routers),
             n_dealer = socket_map_size(l->dealers);
    linq_netw_assert(n_router + n_dealer < MAX_CONNECTIONS);

    memset(items, 0, sizeof(items));
    populate_sockets(l->routers, &ptr);
    populate_sockets(l->dealers, &ptr);

    ptr = items;
    err = zmq_poll(items, n_router + n_dealer, ms);
    if (!(err < 0)) {
        foreach_socket_process_context ctx = { .network = l,
                                               .poll_p = &ptr,
                                               .is_router = true };
        socket_map_foreach(l->routers, foreach_socket_process, &ctx);
        ctx.is_router = false;
        socket_map_foreach(l->dealers, foreach_socket_process, &ctx);
        err = 0;
    }

    // Loop through devices
    device_map_foreach(l->devices, foreach_device_check_request, l);

    // Check if we received a ctrlc and generate an event
    // TODO needs test
    if (!sys_running()) exe_on_ctrlc(l);

    return err;
}

device_s**
linq_netw_device_from_frame(linq_netw_s* l, zframe_t* frame)
{
    char sid[SID_LEN];
    print_null_terminated(sid, SID_LEN, frame);
    return linq_netw_device(l, sid);
}

// get a device from the device map
device_s**
linq_netw_device(const linq_netw_s* l, const char* serial)
{
    return device_map_get(l->devices, serial);
}

// return how many devices are connected to linq
uint32_t
linq_netw_device_count(const linq_netw_s* l)
{
    return device_map_size(l->devices);
}

// Context used for linq_netw_devices_foreach HOF (Higher Order Function)
typedef struct
{
    const linq_netw_s* l;
    linq_netw_devices_foreach_fn fn;
    void* ctx;
} foreach_device_print_sid_ctx;

// linq_netw_device_foreach HOF
static void
foreach_device_print_sid(device_map_s* self, void* ctx, device_s** d_p)
{
    ((void)self);
    foreach_device_print_sid_ctx* foreach_ctx = ctx;
    foreach_ctx->fn(foreach_ctx->ctx, device_serial(*d_p), device_type(*d_p));
}

// Print a list of serial numbers to caller
void
linq_netw_devices_foreach(
    const linq_netw_s* l,
    linq_netw_devices_foreach_fn fn,
    void* ctx)
{
    foreach_device_print_sid_ctx foreach_ctx = { l,
                                                 fn,
                                                 ctx ? ctx : l->context };
    device_map_foreach(l->devices, foreach_device_print_sid, &foreach_ctx);
}

// return how many nodes are connected to linq
uint32_t
linq_netw_nodes_count(const linq_netw_s* l)
{
    return node_map_size(l->nodes);
}

static void
send_error(linq_netw_request_complete_fn fn, void* context, E_LINQ_ERROR e)
{
    char err[32];
    if (fn) {
        snprintf(err, sizeof(err), "{\"error\":%d}", e);
        fn(context, e, err, NULL);
    }
}

// send a get request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_get(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_netw_device(linq, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_get(*d, path, fn, context);
        return LINQ_ERROR_OK;
    }
}

// send a post request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_post(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    const char* json,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_netw_device(linq, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_post(*d, path, json, fn, context);
        return LINQ_ERROR_OK;
    }
}

// send a delete request to a device connected to us
E_LINQ_ERROR
linq_netw_device_send_delete(
    const linq_netw_s* linq,
    const char* serial,
    const char* path,
    linq_netw_request_complete_fn fn,
    void* context)
{
    device_s** d = linq_netw_device(linq, serial);
    if (!d) {
        send_error(fn, context, LINQ_ERROR_DEVICE_NOT_FOUND);
        return LINQ_ERROR_DEVICE_NOT_FOUND;
    } else {
        device_send_delete(*d, path, fn, context);
        return LINQ_ERROR_OK;
    }
}
