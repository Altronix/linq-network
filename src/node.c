#include "node.h"

typedef struct node_s
{
    E_NODE_TYPE type;
    union
    {
        zsock_t* sock;
        zsock_t** ref;
    } socket;
} node_s;

// Connect to a remote rep socket
node_s*
node_connect(const char* ep)
{
    node_s* node = linq_malloc(sizeof(node_s));
    if (node) {
        memset(node, 0, sizeof(node_s));
        node->type = NODE_TYPE_CLIENT;
        node->socket.sock = zsock_new_dealer(ep);
        if (!node->socket.sock) {
            linq_free(node);
            node = NULL;
        }
    }
    return node;
}

void
node_destroy(node_s** node_p)
{
    node_s* node = *node_p;
    *node_p = NULL;
    if (node->type == NODE_TYPE_CLIENT) {
        zsock_destroy(&node->socket.sock);
    } else {
    }
    linq_free(node);
}
