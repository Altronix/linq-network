// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "zmtp.h"
#include "common/base64.h"
#include "common/device.h"
#include "common/json.h"
#include "common/log.h"
#include "common/sys/sys.h"
#include "zmtp_device.h"

#define zmtp_info(...) log_info("ZMTP", __VA_ARGS__)
#define zmtp_warn(...) log_warn("ZMTP", __VA_ARGS__)
#define zmtp_debug(...) log_debug("ZMTP", __VA_ARGS__)
#define zmtp_trace(...) log_trace("ZMTP", __VA_ARGS__)
#define zmtp_error(...) log_error("ZMTP", __VA_ARGS__)
#define zmtp_fatal(...) log_fatal("ZMTP", __VA_ARGS__)

static void
zmq_socket_free_fn(void** socket_p)
{
    zmq_socket_s* s = *socket_p;
    *socket_p = NULL;
    zmq_close(s);
}
MAP_INIT(socket, zmq_socket_s, zmq_socket_free_fn);

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

typedef struct
{
    zmq_msg_t (*msgs)[FRAME_MAX];
    char ver;
    bool router;
} incoming_s;

char g_frame_ver_0 = FRAME_VER_0;
char g_frame_ver_1 = FRAME_VER_1;
char g_frame_typ_heartbeat = FRAME_TYP_HEARTBEAT;
char g_frame_typ_request = FRAME_TYP_REQUEST;
char g_frame_typ_response = FRAME_TYP_RESPONSE;
char g_frame_typ_alert = FRAME_TYP_ALERT;
char g_frame_typ_hello = FRAME_TYP_HELLO;

static int32_t
read_i32(uint8_t* bytes)
{
    int32_t sz = 0;
    for (int i = 0; i < 4; i++) { ((uint8_t*)&sz)[3 - i] = *bytes++; }
    return sz;
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
print_null_terminated(char* c, uint32_t sz, zmq_msg_t* msg)
{
    uint32_t fsz = zmq_msg_size(msg);
    if (fsz + 1 <= sz) {
        memcpy(c, zmq_msg_data(msg), fsz);
        c[fsz] = 0;
        return 0;
    } else {
        return -1;
    }
}

static node_s**
device_get(const zmtp_s* zmtp, const char* serial)
{
    return devices_get(*zmtp->devices_p, serial);
}

node_s**
device_get_from_frame(zmtp_s* l, zmq_msg_t* frame)
{
    char sid[SID_LEN];
    print_null_terminated(sid, SID_LEN, frame);
    return device_get(l, sid);
}

// read incoming zmq frame and test size is equal to expected
zmq_msg_t*
check_eq(zmq_msg_t* msg, uint32_t expect)
{
    return zmq_msg_size(msg) == expect ? msg : NULL;
}

// read incoming zmq frame and test size is less than value
zmq_msg_t*
check_lt(zmq_msg_t* msg, uint32_t lt)
{
    return zmq_msg_size(msg) < lt ? msg : NULL;
}

// read incoming zmq frame and test size is less than value
zmq_msg_t*
check_le(zmq_msg_t* msg, uint32_t le)
{
    return zmq_msg_size(msg) <= le ? msg : NULL;
}

// read incoming zmq frame and test valid json
int
read_alert(zmq_msg_t* msg, netw_alert_s* a)
{
    int count, sz;
    if (check_le(msg, JSON_LEN)) {
        sz = zmq_msg_size(msg);
        memcpy(a->data, zmq_msg_data(msg), sz);
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
            zmtp_error("Failed to parse alert!");
            return -1;
        }
    } else {
        zmtp_error("Alert data to large!");
        return -1;
    }
    return 0;
}

static int
read_email(zmq_msg_t* msg, netw_email_s* emails)
{
    int count, sz;
    if (check_le(msg, JSON_LEN)) {
        sz = zmq_msg_size(msg);
        memcpy(emails->data, zmq_msg_data(msg), sz);
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
            zmtp_error("Failed to parse email");
            return -1;
        }
    } else {
        zmtp_error("Email data to large!");
        return -1;
    }
    return 0;
}

static void
device_legacy_detect(node_s** d, char ver)
{
    const char* sid = device_serial(*d);
    if (ver == FRAME_VER_0 && !device_legacy(*d)) {
        zmtp_debug("[%s] legacy device detected", sid);
        device_legacy_set(*d, true);
    } else if (ver == FRAME_VER_1 && device_legacy(*d)) {
        zmtp_debug("[%s] device updated detected", sid);
        device_legacy_set(*d, false);
    }
}

// A device is resolved by the serial number frame
static node_s**
device_resolve(zmtp_s* l, zmq_socket_s* sock, incoming_s* in, bool insert)
{
    zmq_msg_t* m = *in->msgs;
    uint32_t rid_sz = 0;
    uint8_t* rid = NULL;
    device_map_s* map = *l->devices_p;
    if (in->router) {
        rid_sz = zmq_msg_size(&m[FRAME_RID_IDX]);
        rid = zmq_msg_data(&m[FRAME_RID_IDX]);
    }
    char sid[SID_LEN], tid[TID_LEN] = { 0 };
    print_null_terminated(sid, SID_LEN, &m[FRAME_SID_IDX]);
    print_null_terminated(tid, TID_LEN, &m[FRAME_HB_TID_IDX]);
    zmtp_trace("[%s] Heartbeat received", sid);
    node_s** d = devices_get(map, sid);
    if (d) {
        zmtp_debug("[%s] Device exists", sid);
        device_heartbeat(*d);
        if (rid) zmtp_device_update_router(*d, rid, rid_sz);
        device_legacy_detect(d, in->ver);
    } else {
        if (insert) {
            zmtp_debug("[%s] New device", sid);
            node_s* node = zmtp_device_create(sock, rid, rid_sz, sid, tid);
            if (node) d = devices_add(map, device_serial(node), &node);
            device_legacy_detect(d, in->ver);
            if (l->callbacks && l->callbacks->on_new) {
                zmtp_debug("Executing new callback", sid);
                l->callbacks->on_new(l->context, sid);
            }
        }
    }
    return d;
}

// Node is resolved by grabing object with base64_encoded key of the router id
// If node == router => node = map[router]
// else              => node = seek(map[iter].sock == sock)
static node_zmtp_s**
node_resolve(zmq_socket_s* sock, node_map_s* map, incoming_s* in, bool insert)
{
    zmq_msg_t* m = *in->msgs;
    static uint32_t dealer_key_gen = 0;
    char key[B64_RID_LEN];
    uint32_t ridlen = 0;
    size_t keylen = sizeof(key);
    uint8_t* rid = NULL;
    node_zmtp_s** node = NULL;
    if (&m[FRAME_RID_IDX]) {
        zmtp_debug("node resolve via router");
        ridlen = zmq_msg_size(&m[FRAME_RID_IDX]);
        rid = zmq_msg_data(&m[FRAME_RID_IDX]);
        b64_encode((uint8_t*)key, &keylen, rid, ridlen);
        zmtp_debug("node resolve key [%s] [%d]", key, keylen);
        node = node_map_get(map, key);
        if (!node) {
            zmtp_debug("unknown node (does not exist in hash map)");
            if (insert) {
                zmtp_debug("inserting node into hash map");
                node_zmtp_s* n = node_create(sock, rid, ridlen, key);
                if (n) {
                    node_map_add(map, node_serial(n), &n);
                    node = node_map_get(map, key);
                } else {
                    zmtp_error("failed to create node! [%s]", key);
                }
            }
        }
    } else {
        zmtp_debug("node resolve via seek");
        node = node_map_find_by_sock(map, sock);
        if (!node) {
            zmtp_debug("unknown node (does not exist in hash map)");
            if (insert) {
                zmtp_debug("inserting node into hash map");
                snprintf(key, sizeof(key), "%d", dealer_key_gen++);
                node_zmtp_s* n = node_create(sock, rid, ridlen, key);
                if (n) {
                    node_map_add(map, node_serial(n), &n);
                    node = node_map_get(map, key);
                } else {
                    zmtp_error("failed to create node! [%s]", key);
                }
            }
        }
    }
    return node;
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
// NOTE DEPRECATED
static void
on_device_response(
    void* ctx,
    const char* serial,
    E_LINQ_ERROR error,
    const char* json)
{
    zmtp_trace("Device response received");
    int16_t e = error;
    // TODO use sock_send_frames_n(...)
    node_zmtp_s** node = ctx;
    node_send_frames_n(
        *node,
        5,
        &g_frame_ver_1,        // version
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
process_request(zmtp_s* l, zmq_socket_s* sock, incoming_s* in, uint32_t total)
{
    zmq_msg_t* m = *in->msgs;
    zmtp_trace("Processing request");
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zmq_msg_t *path = NULL, *data = NULL;
    char url[128] = { 0 }, json[JSON_LEN] = { 0 };
    if ((total >= FRAME_REQ_PATH_IDX) &&
        (path = check_le(&m[FRAME_REQ_PATH_IDX], 128))) {
        if (total > FRAME_REQ_DATA_IDX) {
            data = check_le(&m[FRAME_REQ_DATA_IDX], JSON_LEN);
        }
        node_zmtp_s** n = node_resolve(sock, *l->nodes_p, in, false);
        node_s** d = device_get_from_frame(l, &m[FRAME_SID_IDX]);
        if (n && d) {
            print_null_terminated(url, sizeof(url), path);
            if (data) print_null_terminated(json, sizeof(json), data);
            zmtp_trace("Sending request");
            zmtp_device_send_raw(
                *d, url, data ? json : NULL, on_device_response, n);
            e = LINQ_ERROR_OK;
        } else {
            // TODO we arrive here because n == NULL
            //      need to figure how to resolve node (ie use sock)
            //      The node_zmtp_s* is client abstraction to represent a
            //      connection but the sock should be used instead
            zmtp_trace("Device does not exist");
            // TODO send 404 response (device not here)
        }
    }
    zmtp_trace("Request result [%d]", (int)e);
    return e;
}

static E_LINQ_ERROR
parse_response(
    incoming_s* in,
    uint32_t total,
    zmq_msg_t** id,
    zmq_msg_t** e,
    zmq_msg_t** dat)
{
    zmq_msg_t* m = *in->msgs;
    if (in->ver == FRAME_VER_0 && //
        (total >= FRAME_RES_LEGACY_DAT_IDX) &&
        (*e = check_eq(&m[FRAME_RES_LEGACY_ERR_IDX], 2)) &&
        (*dat = check_le(&m[FRAME_RES_LEGACY_DAT_IDX], JSON_LEN))) {
        *id = NULL;
        return LINQ_ERROR_OK;
    } else if (
        in->ver == FRAME_VER_1 && (total >= FRAME_RES_DAT_IDX) &&
        (*id = check_eq(&m[FRAME_RES_ID_IDX], 4)) &&
        (*e = check_eq(&m[FRAME_RES_ERR_IDX], 2)) &&
        (*dat = check_le(&m[FRAME_RES_DAT_IDX], JSON_LEN))) {
        return LINQ_ERROR_OK;
    } else {
        return LINQ_ERROR_PROTOCOL;
    }
}

// check the zmq response frames are valid and process the response
static E_LINQ_ERROR
process_response(zmtp_s* l, zmq_socket_s* sock, incoming_s* in, uint32_t total)
{
    zmtp_trace("Processing response");
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zmq_msg_t *id, *err, *dat, *m = *in->msgs;
    int16_t err_code;
    char json[JSON_LEN] = { 0 };
    if ((e = parse_response(in, total, &id, &err, &dat)) == LINQ_ERROR_OK) {
        node_s** d = device_resolve(l, sock, in, false);
        int32_t reqid = id ? read_i32(zmq_msg_data(id)) : -1;
        if (d) {
            print_null_terminated(json, sizeof(json), dat);
            if (zmtp_device_request_pending(*d, reqid)) {
                err_code = ((uint8_t*)zmq_msg_data(err))[1] |
                           ((uint8_t*)zmq_msg_data(err))[0] << 8;
                if (err_code == LINQ_ERROR_504) {
                    if (zmtp_device_request_retry_count(*d) >=
                        LINQ_NETW_MAX_RETRY) {
                        zmtp_warn(
                            "[%.6s...] (%.3d)", device_serial(*d), err_code);
                        zmtp_device_request_resolve(*d, err_code, json);
                        zmtp_device_request_flush_w_check(*d);
                    } else {
                        zmtp_warn(
                            "[%.6s...] (%.3d) retrying...",
                            device_serial(*d),
                            err_code);
                        // After 504 we don't send retry right away. We wait
                        // a timeout period and then try again
                        zmtp_device_request_retry_at_set(*d, -1);
                    }
                } else {
                    zmtp_debug(
                        "[%.6s...] (%.3d) %.20s...",
                        device_serial(*d),
                        err_code,
                        json);
                    zmtp_device_request_resolve(*d, err_code, json);
                    zmtp_device_request_flush_w_check(*d);
                }
            } else {
                zmtp_warn(
                    "[&.6s...] unsolicited or late response [%d]",
                    device_serial(*d),
                    reqid);
            }
        }
    }
    zmtp_trace("Response result [%d]", (int)e);
    return e;
}

// check the zmq alert frames are valid and process the alert
static E_LINQ_ERROR
process_alert(zmtp_s* z, zmq_socket_s* socket, incoming_s* in, uint32_t total)
{
    zmtp_trace("Processing alert");
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    zmq_msg_t* m = *in->msgs;
    char alert_data[JSON_LEN];
    char email_data[JSON_LEN];
    netw_alert_s alert;
    netw_email_s email;
    memset(&alert, 0, sizeof(alert));
    memset(&email, 0, sizeof(email));
    alert.data = alert_data;
    email.data = email_data;
    if (total >= FRAME_ALERT_DST_IDX &&
        (check_le(&m[FRAME_ALERT_TID_IDX], TID_LEN)) &&
        (!read_alert(&m[FRAME_ALERT_DAT_IDX], &alert)) &&
        (!read_email(&m[FRAME_ALERT_DST_IDX], &email))) {
        node_s** d = device_resolve(z, socket, in, false);
        frames_s f = { .n = 6, .frames = &m[1] };
        if (d) {
            E_TYPE t = ((char*)zmq_msg_data(&m[FRAME_TYP_IDX]))[0];
            if (!FRAME_IS_BROADCAST(t)) {
                FRAME_SET_BROADCAST(t);
                ((char*)zmq_msg_data(&m[FRAME_TYP_IDX]))[0] = t;
                node_map_foreach(*z->nodes_p, foreach_node_forward_message, &f);
            }
            if (z->callbacks && z->callbacks->on_alert) {
                zmtp_trace("Executing alert callback");
                z->callbacks->on_alert(
                    z->context, device_serial(*d), &alert, &email);
            }
            e = LINQ_ERROR_OK;
        }
    }

    zmtp_trace("Alert result [%d]", (int)e);
    return e;
}

// check the zmq hello message is valid and add a node if it does not exist
// TODO nodes are not resolvable from hello packet
static E_LINQ_ERROR
process_hello(zmtp_s* z, zmq_socket_s* socket, incoming_s* in)
{
    zmtp_trace("Processing hello");
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    node_zmtp_s** s = node_resolve(socket, *z->nodes_p, in, true);
    if (s) e = LINQ_ERROR_OK;
    zmtp_trace("Hello result [%d]", (int)e);
    return e;
}

// check the zmq heartbeat frames are valid and process the heartbeat
static E_LINQ_ERROR
process_heartbeat(zmtp_s* z, zmq_socket_s* s, incoming_s* in, uint32_t total)
{
    zmtp_trace("Processing heartbeat");
    zmq_msg_t* m = *in->msgs;
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    if (total >= FRAME_HB_SITE_IDX && //
        (check_le(&m[FRAME_HB_TID_IDX], TID_LEN)) &&
        (check_le(&m[FRAME_HB_SITE_IDX], SITE_LEN))) {
        frames_s f = { .n = 5, .frames = &m[1] };
        node_s** d = device_resolve(z, s, in, true);
        if (d) {
            zmtp_debug("Device resolved");
            E_TYPE t = ((char*)zmq_msg_data(&m[FRAME_TYP_IDX]))[0];
            if (!FRAME_IS_BROADCAST(t)) {
                FRAME_SET_BROADCAST(t);
                ((char*)zmq_msg_data(&m[FRAME_TYP_IDX]))[0] = t;
                node_map_foreach(*z->nodes_p, foreach_node_forward_message, &f);
            }
            if (z->callbacks && z->callbacks->on_heartbeat) {
                zmtp_debug("Executing heartbeat callback");
                z->callbacks->on_heartbeat(z->context, device_serial(*d));
            }
            e = LINQ_ERROR_OK;
        }
    }
    zmtp_trace("Heartbeat result [%d]", (int)e);
    return e;
}

// check the zmq header frames are valid and process the packet
static E_LINQ_ERROR
process_packet(zmtp_s* z, zmq_socket_s* s, bool router)
{
    zmtp_trace("Processing packet");
    E_LINQ_ERROR e = LINQ_ERROR_PROTOCOL;
    int more = 1, err, start = router ? FRAME_RID_IDX : FRAME_VER_IDX,
        end = start;
    char sid[SID_LEN] = "", ver;
    zmq_msg_t m[FRAME_MAX];
    while (more) {
        linq_network_assert(end < FRAME_MAX);
        err = zmq_msg_init(&m[end]);
        linq_network_assert(err == 0);
        err = zmq_msg_recv(&m[end], s, 0);
        linq_network_assert(!(err == -1));
        more = zmq_msg_more(&m[end]);
        end++;
    }

    if (end >= 3 && (!router || (router && check_le(&m[0], RID_LEN))) &&
        (check_eq(&m[FRAME_VER_IDX], 1)) && (check_eq(&m[FRAME_TYP_IDX], 1)) &&
        (check_le(&m[FRAME_SID_IDX], SID_LEN))) {
        ver = ((char*)zmq_msg_data(&m[FRAME_VER_IDX]))[0];
        E_TYPE t = FRAME_TYPE(((uint8_t*)zmq_msg_data(&m[FRAME_TYP_IDX]))[0]);
        incoming_s in = { .router = router, .msgs = &m, .ver = ver };
        switch (t) {
            case TYPE_HEARTBEAT: e = process_heartbeat(z, s, &in, end); break;
            case TYPE_REQUEST: e = process_request(z, s, &in, end); break;
            case TYPE_RESPONSE: e = process_response(z, s, &in, end); break;
            case TYPE_ALERT: e = process_alert(z, s, &in, end); break;
            case TYPE_HELLO: e = process_hello(z, s, &in); break;
        }
    }
    if (e) {
        zmtp_error("Processing packet error [%d]", (int)e);
        if (z->callbacks && z->callbacks->on_err) {
            zmtp_trace("Executing error callback");
            if (end >= FRAME_SID_IDX) {
                print_null_terminated(sid, SID_LEN, &m[FRAME_SID_IDX]);
            }
            z->callbacks->on_err(z->context, e, sid, "");
        }
    }
    for (int i = start; i < end; i++) { zmq_msg_close(&m[i]); }
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
    zmtp->zmq = zmq_ctx_new();
    zmtp->devices_p = devices_p;
    zmtp->nodes_p = nodes_p;
    zmtp->callbacks = callbacks;
    zmtp->context = context;
    zmtp->shutdown = false;
    zmtp->routers = socket_map_create();
    zmtp->dealers = socket_map_create();
    zmtp_debug("Context created...");
}

void
zmtp_deinit(zmtp_s* zmtp)
{
    socket_map_destroy(&zmtp->routers);
    socket_map_destroy(&zmtp->dealers);
    zmq_ctx_destroy(zmtp->zmq);
    zmtp->zmq = NULL;
    zmtp_debug("Context destroyed...");
}

netw_socket
zmtp_listen(zmtp_s* zmtp, const char* ep)
{
    int err;
    zmq_socket_s* socket = zmq_socket(zmtp->zmq, ZMQ_ROUTER);
    linq_network_assert(socket);
    err = zmq_bind(socket, ep);
    if (err == 0) {
        socket_map_add(zmtp->routers, ep, &socket);
        int key = socket_map_key(zmtp->routers, ep);
        key |= ATX_NET_SOCKET_TYPE_ROUTER << 0x08;
        zmtp_info("bind socket success [%s]", ep);
        return key;
    } else {
        zmq_close(socket);
        zmtp_error("failed to bind socket [%s]", ep);
        return LINQ_ERROR_SOCKET;
    }
}

netw_socket
zmtp_connect(zmtp_s* zmtp, const char* ep)
{
    int err;
    zmq_socket_s* socket = zmq_socket(zmtp->zmq, ZMQ_DEALER);
    linq_network_assert(socket);
    err = zmq_connect(socket, ep);
    if (err == 0) {
        socket = *socket_map_add(zmtp->dealers, ep, &socket);
        node_zmtp_s* n = node_create(socket, NULL, 0, ep);
        linq_network_assert(n);
        node_send_hello(n);
        node_map_add(*zmtp->nodes_p, node_serial(n), &n);
        int key = socket_map_key(zmtp->dealers, ep);
        key |= ATX_NET_SOCKET_TYPE_DEALER << 0x08;
        return key;
    } else {
        zmq_close(socket);
        zmtp_error("failed to connect [%s]", ep);
        return LINQ_ERROR_SOCKET;
    }
}

static bool
remove_if(node_s* v, void* socket)
{
    if (v->transport == TRANSPORT_ZMTP) {
        netw_socket_s* s = (netw_socket_s*)v;
        if (s->sock == socket) return true;
    }
    return false;
}

E_LINQ_ERROR
zmtp_close_router(zmtp_s* zmtp, netw_socket handle)
{
    int socket = ATX_NET_SOCKET(handle), count;
    zmq_socket_s** s = socket_map_resolve(zmtp->routers, socket);
    if (s) {
        // remove_devices(s, *zmtp->devices_p);
        count = devices_foreach_remove_if(*zmtp->devices_p, remove_if, s);
        zmtp_info("[%d] device nodes closed");
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
    zmq_socket_s** s = socket_map_resolve(zmtp->dealers, socket);
    if (s) {
        // remove_devices(s, *zmtp->devices_p);
        // remove_nodes(s, *zmtp->nodes_p);
        count = devices_foreach_remove_if(*zmtp->devices_p, remove_if, s);
        zmtp_info("[%d] device nodes closed");
        count = node_map_foreach_remove_if(*zmtp->nodes_p, remove_if, s);
        zmtp_info("[%d] client nodes closed");
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
            ptr->socket = map_val(hash, iter);                                 \
            ptr->events = ZMQ_POLLIN;                                          \
            ptr++;                                                             \
        }                                                                      \
    }
#define process_sockets(err, zmtp, hash, iter, ptr, router)                    \
    map_foreach(hash, iter)                                                    \
    {                                                                          \
        if (map_has_key(hash, iter)) {                                         \
            if (ptr->revents && ZMQ_POLLIN) {                                  \
                err = process_packet(zmtp, map_val(hash, iter), router);       \
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
        process_sockets(err, zmtp, zmtp->routers, iter, ptr, true);
        process_sockets(err, zmtp, zmtp->dealers, iter, ptr, false);
        err = 0;
    }

    // Check if we received a ctrlc and generate an event
    // TODO needs test
    if (!sys_running()) {
        if (zmtp->callbacks && zmtp->callbacks->on_ctrlc) {
            zmtp_trace("Executing ctrlc callback");
            zmtp->callbacks->on_ctrlc(zmtp->context);
        }
    }

    return err;
}

bool
sys_running()
{
    // TODO
    return true;
    // return !zsys_is_interrupted();
}
