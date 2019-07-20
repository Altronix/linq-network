#include "node.h"

typedef struct node_s
{
    E_NODE_TYPE type;
    router_s router;
    zsock_t* sock;
    zsock_t** sock_p;
} node_s;

// Connect to a remote rep socket
node_s*
node_connect(const char* ep)
{
    node_s* node = linq_malloc(sizeof(node_s));
    if (node) {
        memset(node, 0, sizeof(node_s));
        node->type = NODE_TYPE_CLIENT;
        node->sock_p = &node->sock;
        node->sock = zsock_new_dealer(ep);
        if (!node->sock) {
            linq_free(node);
            node = NULL;
        }
    }
    return node;
}

node_s*
node_recv(zsock_t** sock_p, uint8_t* router, uint32_t router_sz)
{
    node_s* node = linq_malloc(sizeof(node_s));
    if (node) {
        memset(node, 0, sizeof(node_s));
        node->type = NODE_TYPE_SERVER;
        node->sock_p = sock_p;
        node_update_router(node, router, router_sz);
    }
    return node;
}

void
node_update_router(node_s* node, uint8_t* router, uint32_t router_sz)
{
    memcpy(&node->router.id, router, router_sz);
    node->router.sz = router_sz;
}

void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    *node_p = NULL;
    if (node->sock) zsock_destroy(&node->sock);
    linq_free(node);
}
