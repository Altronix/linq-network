// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zmtp.h"
#include "base64.h"
#include "device.h"
#include "log.h"
#include "sys.h"
#include "zmtp_device.h"

#include "json.h"

extern volatile int zsys_interrupted;

MAP_INIT(socket, zsock_t, zsock_destroy);

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

char g_frame_ver_0 = FRAME_VER_0;
char g_frame_typ_heartbeat = FRAME_TYP_HEARTBEAT;
char g_frame_typ_request = FRAME_TYP_REQUEST;
char g_frame_typ_response = FRAME_TYP_RESPONSE;
char g_frame_typ_alert = FRAME_TYP_ALERT;
char g_frame_typ_hello = FRAME_TYP_HELLO;

static bool
is_router(zsock_t* sock)
{
    return zsock_type_str(sock)[0] == 'R';
}

static atx_str
json_get_str(const char* b, const jsontok* t)
{
    atx_str ret;
    ret.p = &b[t->start];
    ret.len = t->end - t->start;
    return ret;
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

static node_s**
device_get(const zmtp_s* zmtp, const char* serial)
{
    return device_map_get(*zmtp->devices_p, serial);
}

node_s**
device_get_from_frame(zmtp_s* l, zframe_t* frame)
{
    char sid[SID_LEN];
    print_null_terminated(sid, SID_LEN, frame);
    return device_get(l, sid);
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
pop_alert(zmsg_t* msg, netw_alert_s* a)
{
    int count, sz;
    zframe_t* f = pop_le(msg, JSON_LEN);
    sz = zframe_size(f);
    memcpy(a->data, zframe_data(f), sz);
    const jsontok *who, *what, *site, *when, *mesg;
    jsontok t[30];
    json_parser p;
    json_init(&p);
    count = json_parse(&p, a->data, sz, t, 30);
    if (count > 0 && (who = json_get_member(a->data, t, "who")) &&
        (what = json_get_member(a->data, t, "what")) &&
        (site = json_get_member(a->data, t, "siteId")) &&
        (when = json_get_member(a->data, t, "when")) &&
        (mesg = json_get_member(a->data, t, "mesg"))) {
        a->who = json_get_str(a->data, who);
        a->what = json_get_str(a->data, what);
        a->where = json_get_str(a->data, site);
        a->when = json_get_str(a->data, when);
        a->mesg = json_get_str(a->data, mesg);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

static zframe_t*
pop_email(zmsg_t* msg, netw_email_s* emails)
{
    int count, sz;
    zframe_t* f = pop_le(msg, JSON_LEN);
    sz = zframe_size(f);
    memcpy(emails->data, zframe_data(f), sz);
    json_parser p;
    jsontok t[30];
    const jsontok *to0, *to1, *to2, *to3, *to4;
    json_init(&p);
    count = json_parse(&p, emails->data, sz, t, 30);
    if (count > 0 && (to0 = json_get_member(emails->data, t, "to0")) &&
        (to1 = json_get_member(emails->data, t, "to1")) &&
        (to2 = json_get_member(emails->data, t, "to2")) &&
        (to3 = json_get_member(emails->data, t, "to3")) &&
        (to4 = json_get_member(emails->data, t, "to4"))) {
        emails->to0 = json_get_str(emails->data, to0);
        emails->to1 = json_get_str(emails->data, to1);
        emails->to2 = json_get_str(emails->data, to2);
        emails->to3 = json_get_str(emails->data, to3);
        emails->to4 = json_get_str(emails->data, to4);
    } else {
        zframe_destroy(&f);
    }
    return f;
}

// A device is resolved by the serial number frame
static node_s**
device_resolve(zmtp_s* l, zsock_t* sock, zframe_t** frames, bool insert)
{
    uint32_t rid_sz = 0;
    uint8_t* rid = NULL;
    device_map_s* map = *l->devices_p;
    if (frames[FRAME_RID_IDX]) {
        rid_sz = zframe_size(frames[FRAME_RID_IDX]);
        rid = zframe_data(frames[FRAME_RID_IDX]);
    }
    char sid[SID_LEN], tid[TID_LEN] = { 0 };
    print_null_terminated(sid, SID_LEN, frames[FRAME_SID_IDX]);
    print_null_terminated(tid, TID_LEN, frames[FRAME_HB_TID_IDX]);
    node_s** d = device_map_get(map, sid);
    if (d) {
        zmtp_device_heartbeat(*d);
        if (rid) zmtp_device_update_router(*d, rid, rid_sz);
    } else {
        if (insert) {
            node_s* node = zmtp_device_create(sock, rid, rid_sz, sid, tid);
            if (node) d = device_map_add(map, zmtp_device_serial(node), &node);
            if (l->callbacks && l->callbacks->on_new) {
                l->callbacks->on_new(l->context, sid);
            }
        }
    }
    return d;
}

// Node is resolved by grabing object with base64_encoded key of the router id
static node_zmtp_s**
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
    node_zmtp_s** d = node_map_get(map, sid);
    if (d) {
        if (rid) node_update_router(*d, rid, rid_len);
    } else {
        if (insert) {
            node_zmtp_s* node = node_create(sock, rid, rid_len, sid);
            if (node) d = node_map_add(map, node_serial(node), &node);
        }
    }
    return d;
}

// Broadcast x frames to each node (router frame is added per each node)
static void
foreach_node_forward_message(
    node_map_s* self,
    void* ctx,
    const char* key,
    node_zmtp_s** n)
{
    ((void)self);
    ((void)key);
    frames_s* frames = ctx;
    node_send_frames(*n, frames->n, frames->frames);
}

// A device has responded to a request from a node on linq->nodes. Forward
// the response to the node @ linq->nodes
static void
on_device_response(
    void* ctx,
    const char* serial,
    E_LINQ_ERROR error,
    const char* json)
{
    int16_t e = error;
    node_zmtp_s** node = ctx;
    node_send_frames_n(
        *node,
        5,
        &g_frame_ver_0,        // version
        1,                     //
        &g_frame_typ_response, // type
        1,                     //
        serial,                // serial
        strlen(serial),        //
        &e,                    // error
        2,                     //
        json,                  // data
        strlen(json)           //
    );
}

// check the zmq request frames are valid and process the request
static E_LINQ_ERROR
process_request(zmtp_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *path = NULL, *data = NULL;
    char url[128] = { 0 }, json[JSON_LEN] = { 0 };
    if ((zmsg_size(*msg) >= 1) &&
        (path = frames[FRAME_REQ_PATH_IDX] = pop_le(*msg, 128))) {
        if (zmsg_size(*msg) == 1) {
            data = frames[FRAME_REQ_DATA_IDX] = pop_le(*msg, JSON_LEN);
        }
        node_zmtp_s** n = node_resolve(sock, *l->nodes_p, frames, false);
        node_s** d = device_get_from_frame(l, frames[FRAME_SID_IDX]);
        if (n && d) {
            print_null_terminated(url, sizeof(url), path);
            if (data) print_null_terminated(json, sizeof(json), data);
            zmtp_device_send_raw(
                *d, url, data ? json : NULL, on_device_response, n);
            e = LINQ_ERROR_OK;
        } else {
            // TODO send 404 response (device not here)
        }
    }
    return e;
}

// check the zmq response frames are valid and process the response
static E_LINQ_ERROR
process_response(zmtp_s* l, zsock_t* sock, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zframe_t *err, *dat;
    int16_t err_code;
    char json[JSON_LEN] = { 0 };
    if ((zmsg_size(*msg) == 2) &&
        (err = frames[FRAME_RES_ERR_IDX] = pop_eq(*msg, 2)) &&
        (dat = frames[FRAME_RES_DAT_IDX] = pop_le(*msg, JSON_LEN))) {
        e = LINQ_ERROR_OK;
        node_s** d = device_resolve(l, sock, frames, false);
        if (d) {
            print_null_terminated(json, sizeof(json), dat);
            if (zmtp_device_request_pending(*d)) {
                err_code = zframe_data(err)[1] | zframe_data(err)[0] << 8;
                if (err_code == LINQ_ERROR_504) {
                    if (zmtp_device_request_retry_count(*d) >=
                        LINQ_NETW_MAX_RETRY) {
                        log_warn(
                            "(ZMTP) [%.6s...] (%.3d)",
                            zmtp_device_serial(*d),
                            err_code);
                        zmtp_device_request_resolve(*d, err_code, json);
                        zmtp_device_request_flush_w_check(*d);
                    } else {
                        log_warn(
                            "(ZMTP) [%.6s...] (%.3d) retrying...",
                            zmtp_device_serial(*d),
                            err_code);
                        uint32_t retry = sys_tick() + LINQ_NETW_RETRY_TIMEOUT;
                        zmtp_device_request_retry_at_set(*d, retry);
                    }
                } else {
                    log_debug(
                        "(ZMTP) [%.6s...] (%.3d) %.20s...",
                        device_serial(*d),
                        err_code,
                        json);
                    zmtp_device_request_resolve(*d, err_code, json);
                    zmtp_device_request_flush_w_check(*d);
                }
            }
        }
    }
    return e;
}

// check the zmq alert frames are valid and process the alert
static E_LINQ_ERROR
process_alert(zmtp_s* z, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    char alert_data[JSON_LEN];
    char email_data[JSON_LEN];
    netw_alert_s alert;
    netw_email_s email;
    memset(&alert, 0, sizeof(alert));
    memset(&email, 0, sizeof(email));
    alert.data = alert_data;
    email.data = email_data;
    if (zmsg_size(*msg) == 3 &&
        (frames[FRAME_ALERT_TID_IDX] = pop_le(*msg, TID_LEN)) &&
        (frames[FRAME_ALERT_DAT_IDX] = pop_alert(*msg, &alert)) &&
        (frames[FRAME_ALERT_DST_IDX] = pop_email(*msg, &email))) {
        node_s** d = device_resolve(z, socket, frames, false);
        frames_s f = { 6, &frames[1] };
        if (d) {
            if (zmtp_device_no_hops(*d)) {
                // We only broadcast when the device is directly connected
                // otherwize, nodes would rebroadcast to eachother infinite
                node_map_foreach(*z->nodes_p, foreach_node_forward_message, &f);
            }
            if (z->callbacks && z->callbacks->on_alert) {
                z->callbacks->on_alert(
                    z->context, zmtp_device_serial(*d), &alert, &email);
            }
            e = LINQ_ERROR_OK;
        }
    }

    return e;
}

// check the zmq hello message is valid and add a node if it does not exist
static E_LINQ_ERROR
process_hello(zmtp_s* z, zsock_t* socket, zmsg_t** msg, zframe_t** frames)
{
    ((void)msg);
    ((void)socket);
    ((void)frames);
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    node_zmtp_s** s = node_resolve(socket, *z->nodes_p, frames, true);
    if (s) e = LINQ_ERROR_OK;
    return e;
}

// check the zmq heartbeat frames are valid and process the heartbeat
static E_LINQ_ERROR
process_heartbeat(zmtp_s* z, zsock_t* s, zmsg_t** msg, zframe_t** frames)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    if (zmsg_size(*msg) == 2 &&
        (frames[FRAME_HB_TID_IDX] = pop_le(*msg, TID_LEN)) &&
        (frames[FRAME_HB_SITE_IDX] = pop_le(*msg, SITE_LEN))) {
        node_s** d = device_resolve(z, s, frames, true);
        frames_s f = { 5, &frames[1] };
        if (d) {
            if (zmtp_device_no_hops(*d)) {
                // We only broadcast when the device is directly connected
                // otherwize, nodes would rebroadcast to eachother infinite
                node_map_foreach(*z->nodes_p, foreach_node_forward_message, &f);
            }
            if (z->callbacks && z->callbacks->on_heartbeat) {
                z->callbacks->on_heartbeat(z->context, zmtp_device_serial(*d));
            }
            e = LINQ_ERROR_OK;
        }
    }
    return e;
}

// check the zmq header frames are valid and process the packet
static E_LINQ_ERROR
process_packet(zmtp_s* z, zsock_t* s)
{
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    int total_frames = 0, start = 1;
    bool router = is_router(s);
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
            case TYPE_HEARTBEAT: e = process_heartbeat(z, s, &msg, f); break;
            case TYPE_REQUEST: e = process_request(z, s, &msg, f); break;
            case TYPE_RESPONSE: e = process_response(z, s, &msg, f); break;
            case TYPE_ALERT: e = process_alert(z, s, &msg, f); break;
            case TYPE_HELLO: e = process_hello(z, s, &msg, f); break;
        }
    }
    if (e) {
        if (z->callbacks && z->callbacks->on_err) {
            z->callbacks->on_err(z->context, e, "", "");
        }
    }
    zmsg_destroy(&msg);
    for (int i = 0; i < total_frames; i++) {
        if (f[start]) zframe_destroy(&f[start]);
        start++;
    }
    return e;
}

void
zmtp_init(
    zmtp_s* zmtp,
    device_map_s** devices_p,
    node_map_s** nodes_p,
    const netw_callbacks* callbacks,
    void* context)
{
    zmtp->devices_p = devices_p;
    zmtp->nodes_p = nodes_p;
    zmtp->callbacks = callbacks;
    zmtp->context = context;
    zmtp->shutdown = false;
    zmtp->routers = socket_map_create();
    zmtp->dealers = socket_map_create();
    log_debug("(ZMTP) Context created...");
}

void
zmtp_deinit(zmtp_s* zmtp)
{
    socket_map_destroy(&zmtp->routers);
    socket_map_destroy(&zmtp->dealers);
    log_debug("(ZMTP) Context destroyed...");
}

netw_socket
zmtp_listen(zmtp_s* zmtp, const char* ep)
{
    zsock_t* socket = zsock_new_router(ep);
    if (socket) {
        socket_map_add(zmtp->routers, ep, &socket);
        int key = socket_map_key(zmtp->routers, ep);
        key |= ATX_NET_SOCKET_TYPE_ROUTER << 0x08;
        return key;
    } else {
        return LINQ_ERROR_SOCKET;
    }
}

netw_socket
zmtp_connect(zmtp_s* zmtp, const char* ep)
{
    zsock_t* socket = zsock_new_dealer(ep);
    if (socket) {
        socket_map_add(zmtp->dealers, ep, &socket);
        node_zmtp_s* n =
            node_create(*socket_map_get(zmtp->dealers, ep), NULL, 0, ep);
        if (n) {
            node_send_hello(n);
            node_map_add(*zmtp->nodes_p, ep, &n);
        }
        int key = socket_map_key(zmtp->dealers, ep);
        key |= ATX_NET_SOCKET_TYPE_DEALER << 0x08;
        return key;
    }
    return LINQ_ERROR_SOCKET;
}

E_LINQ_ERROR
zmtp_close_router(zmtp_s* zmtp, netw_socket handle)
{
    int socket = ATX_NET_SOCKET(handle), count;
    zsock_t** s = socket_map_resolve(zmtp->routers, socket);
    if (s) {
        // remove_devices(s, *zmtp->devices_p);
        count = device_map_foreach_remove_if_sock_eq(*zmtp->devices_p, *s);
        log_info("(ZMTP) [%d] device nodes closed");
        socket_map_remove_iter(zmtp->routers, socket);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

E_LINQ_ERROR
zmtp_close_dealer(zmtp_s* zmtp, netw_socket handle)
{
    int socket = ATX_NET_SOCKET(handle), count;
    zsock_t** s = socket_map_resolve(zmtp->dealers, socket);
    if (s) {
        // remove_devices(s, *zmtp->devices_p);
        // remove_nodes(s, *zmtp->nodes_p);
        count = device_map_foreach_remove_if_sock_eq(*zmtp->devices_p, *s);
        log_info("(ZMTP) [%d] device nodes closed");
        count = node_foreach_remove_if_sock_eq(*zmtp->nodes_p, *s);
        log_info("(ZMTP) [%d] client nodes closed");
        socket_map_remove_iter(zmtp->dealers, socket);
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_BAD_ARGS;
    }
}

#define populate_sockets(hash, iter, ptr)                                      \
    map_foreach(hash, iter)                                                    \
    {                                                                          \
        if (map_has_key(hash, iter)) {                                         \
            ptr->socket = zsock_resolve(map_val(hash, iter));                  \
            ptr->events = ZMQ_POLLIN;                                          \
            ptr++;                                                             \
        }                                                                      \
    }
#define process_sockets(err, zmtp, hash, iter, ptr)                            \
    map_foreach(hash, iter)                                                    \
    {                                                                          \
        if (map_has_key(hash, iter)) {                                         \
            if (ptr->revents && ZMQ_POLLIN) {                                  \
                err = process_packet(zmtp, map_val(hash, iter));               \
            }                                                                  \
            ptr++;                                                             \
        }                                                                      \
    }
E_LINQ_ERROR
zmtp_poll(zmtp_s* zmtp, int32_t ms)
{
    map_iter iter;
    zmq_pollitem_t items[MAX_CONNECTIONS], *ptr = items;
    int err, n_router = socket_map_size(zmtp->routers),
             n_dealer = socket_map_size(zmtp->dealers);
    linq_network_assert(n_router + n_dealer < MAX_CONNECTIONS);
    memset(items, 0, sizeof(items));

    ptr = items;
    populate_sockets(zmtp->routers, iter, ptr);
    populate_sockets(zmtp->dealers, iter, ptr);

    err = zmq_poll(items, n_router + n_dealer, ms);
    if (!(err < 0)) {
        ptr = items;
        process_sockets(err, zmtp, zmtp->routers, iter, ptr);
        process_sockets(err, zmtp, zmtp->dealers, iter, ptr);
        err = 0;
    }

    // Check if we received a ctrlc and generate an event
    // TODO needs test
    if (!sys_running()) {
        if (zmtp->callbacks && zmtp->callbacks->on_ctrlc) {
            zmtp->callbacks->on_ctrlc(zmtp->context);
        }
    }

    return err;
}

bool
sys_running()
{
    return !zsys_interrupted;
}
