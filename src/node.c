#include "node.h"
#include "containers.h"

typedef struct node_s
{
    zsock_t* sock;
    router_s router;
    char serial[SID_LEN];
} node_s;

node_s*
node_create(
    zsock_t* s,
    const uint8_t* router,
    uint32_t router_sz,
    const char* sid)
{
    node_s* node = linq_malloc(sizeof(node_s));
    if (node) {
        memset(node, 0, sizeof(node_s));
        node->sock = s;
        node_update_router(node, router, router_sz);
        snprintf(node->serial, sizeof(node->serial), "%s", sid);
    }
    return node;
}

void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    *node_p = NULL;
    memset(node, 0, sizeof(node_s));
    linq_free(node);
}

void
node_update_router(node_s* node, const uint8_t* r, uint32_t sz)
{
    memcpy(&node->router.id, r, sz);
    node->router.sz = sz;
}

const char*
node_serial(node_s* node)
{
    return node->serial;
}

void
node_send_frames(node_s* node, uint32_t n, zframe_t** frames)
{

    uint32_t count = 0;
    zmsg_t* msg = zmsg_new();
    if (msg) {
        zframe_t* router = zframe_new(node->router.id, node->router.sz);
        if (router) {
            zmsg_append(msg, &router);
            for (uint32_t i = 0; i < n; i++) {
                zframe_t* frame = zframe_dup(frames[i]);
                if (!frame) break;
                count++;
                zmsg_append(msg, &frame);
            }
            if (count == n) {
                int err = zmsg_send(&msg, node->sock);
                if (err) zmsg_destroy(&msg);
            }
        }
    }
}

void
node_send_frames_n(node_s* node, uint32_t n, ...)
{
    int err = -1;
    uint32_t count = 0;
    zmsg_t* msg = zmsg_new();
    if (msg) {
        zframe_t* router = zframe_new(node->router.id, node->router.sz);
        if (router) {
            zmsg_append(msg, &router);
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
            if (count == n) err = zmsg_send(&msg, node->sock);
        }
        if (err) zmsg_destroy(&msg);
    }
}
