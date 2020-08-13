// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "node.h"
#include "containers.h"

// main class struct (extends netw_socket_s)
typedef struct node_zmtp_s
{
    node_s base;     // will cast into netw_socket_s ...must be on top
    zsock_t* sock;   // will cast into netw_socket_s ...must be on top
    router_s router; // will cast into netw_socket_s ...must be on top
    char serial[SID_LEN];
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
        if (router_sz) node_update_router(node, router, router_sz);
        if (sid) snprintf(node->serial, sizeof(node->serial), "%s", sid);
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
    return node->serial;
}

static int
optional_prepend_router(node_zmtp_s* node, zmsg_t* msg)
{
    if (node->router.sz) {
        zframe_t* router = zframe_new(node->router.id, node->router.sz);
        if (router) {
            zmsg_prepend(msg, &router);
        } else {
            zmsg_destroy(&msg);
            return -1;
        }
    }
    return 0;
}

void
node_send_hello(node_zmtp_s* node)
{
    const char* sid = node_serial(node);
    node_send_frames_n(
        node, 3, &g_frame_ver_0, 1, &g_frame_typ_hello, 1, sid, strlen(sid));
}

void
node_send_frames(node_zmtp_s* node, uint32_t n, zframe_t** frames)
{

    uint32_t count = 0;
    zmsg_t* msg = zmsg_new();

    if (msg) {
        for (uint32_t i = 0; i < n; i++) {
            zframe_t* frame = zframe_dup(frames[i]);
            if (!frame) break;
            count++;
            zmsg_append(msg, &frame);
        }
        if (!(count == n && !optional_prepend_router(node, msg) &&
              !zmsg_send(&msg, node->sock))) {
            zmsg_destroy(&msg);
        }
    }
}

void
node_send_frames_n(node_zmtp_s* node, uint32_t n, ...)
{
    uint32_t count = 0;
    zmsg_t* msg = zmsg_new();

    if (msg) {
        va_list list;
        va_start(list, n);
        for (uint32_t i = 0; i < n; i++) {
            uint8_t* arg = va_arg(list, uint8_t*);
            size_t sz = va_arg(list, size_t);
            zframe_t* f = zframe_new(arg, sz);
            if (f) {
                count++;
                zmsg_append(msg, &f);
            }
        }
        va_end(list);

        if (!(count == n && !optional_prepend_router(node, msg) &&
              !zmsg_send(&msg, node->sock))) {
            zmsg_destroy(&msg);
        }
    }
}
