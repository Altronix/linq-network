// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "node.h"
#include "containers.h"
#include "log.h"
#include "zmtp.h"

// main class struct (extends netw_socket_s)
typedef struct node_zmtp_s
{
    node_s base;     // will cast into netw_socket_s ...must be on top
    zsock_t* sock;   // will cast into netw_socket_s ...must be on top
    router_s router; // will cast into netw_socket_s ...must be on top
} node_zmtp_s;
MAP_INIT(node, node_zmtp_s, node_destroy);

node_zmtp_s*
node_create(
    zsock_t* s,
    const uint8_t* router,
    uint32_t router_sz,
    const char* sid)
{
    node_zmtp_s* node = linq_network_malloc(sizeof(node_zmtp_s));
    if (node) {
        memset(node, 0, sizeof(node_zmtp_s));
        node->sock = s;
        node->base.transport = TRANSPORT_ZMTP;
        if (router_sz) node_update_router(node, router, router_sz);
        if (sid) {
            snprintf(node->base.serial, sizeof(node->base.serial), "%s", sid);
        }
    }
    return node;
}

void
node_destroy(node_zmtp_s** node_p)
{
    node_zmtp_s* node = *node_p;
    *node_p = NULL;
    memset(node, 0, sizeof(node_zmtp_s));
    linq_network_free(node);
}

void
node_update_router(node_zmtp_s* node, const uint8_t* r, uint32_t sz)
{
    memcpy(&node->router.id, r, sz);
    node->router.sz = sz;
}

const char*
node_serial(node_zmtp_s* node)
{
    return node->base.serial;
}

static bool
optional_send_router(node_zmtp_s* node)
{
    zmq_msg_t m;
    int err;
    if (node->router.sz) {
        err = zmq_msg_init_size(&m, node->router.sz);
        linq_network_assert(err == 0);
        memcpy(zmq_msg_data(&m), node->router.id, node->router.sz);
        err = zmq_msg_send(&m, zsock_resolve(node->sock), ZMQ_SNDMORE);
        linq_network_assert(err >= 0);
        return true;
    } else {
        return false;
    }
}

void
node_send_hello(node_zmtp_s* node)
{
    const char* sid = node_serial(node);
    node_send_frames_n(
        node, 3, &g_frame_ver_0, 1, &g_frame_typ_hello, 1, sid, strlen(sid));
}

void
node_send_frames(node_zmtp_s* node, uint32_t n, zmq_msg_t* frames)
{

    zmq_msg_t m;
    int err;
    uint32_t count = 0;

    optional_send_router(node);

    while (count < n) {
        zmq_msg_init(&m);
        err = zmq_msg_copy(&m, &frames[count++]);
        linq_network_assert(err == 0);
        err = zmq_msg_send(
            &m, zsock_resolve(node->sock), count == (n - 1) ? 0 : ZMQ_SNDMORE);
        if (err < 0) {
            log_error("NODE", "Failed to send to node!");
            zmq_msg_close(&m);
            break;
        }
    }
}

void
node_send_frames_n(node_zmtp_s* node, uint32_t n, ...)
{
    zmq_msg_t m;
    int err;
    uint32_t count = 0;

    optional_send_router(node);

    va_list list;
    va_start(list, n);
    while (count < n) {
        uint8_t* arg = va_arg(list, uint8_t*);
        size_t sz = va_arg(list, size_t);
        err = zmq_msg_init_size(&m, sz);
        linq_network_assert(err == 0);
        memcpy(zmq_msg_data(&m), arg, sz);
        err = zmq_msg_send(
            &m, zsock_resolve(node->sock), count == n - 1 ? 0 : ZMQ_SNDMORE);
        if (err < 0) {
            log_error("NODE", "Failed to send to node!");
            zmq_msg_close(&m);
            break;
        }
        count++;
    }
    va_end(list);
}

uint32_t
node_map_foreach_remove_if(
    node_map_s* hash,
    bool (*remove)(node_s*, void*),
    void* ctx)
{
    uint32_t n = 0;
    map_iter iter;
    node_s* v;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter) && (v = (node_s*)map_val(hash, iter))) {
            if (remove(v, ctx)) {
                node_map_remove(hash, v->serial);
                n++;
            }
        }
    }
    return n;
}

node_zmtp_s**
node_map_find_by_sock(node_map_s* hash, zsock_t* sock)
{
    map_iter iter;
    map_foreach(hash, iter)
    {
        if (map_has_key(hash, iter)) {
            if (map_val(hash, iter)->sock == sock) return &map_val(hash, iter);
        }
    }
    return NULL;
}
