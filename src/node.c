#include "node.h"

typedef struct node_s
{
    zsock_t* sock;
} node_s;

// Connect to a remote rep socket
node_s*
node_connect(const char* ep)
{
    node_s* node = linq_malloc(sizeof(node_s));
    if (node) {
        memset(node, 0, sizeof(node_s));
        node->sock = zsock_new_dealer(ep);
        if (!node->sock) {
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
    zsock_destroy(&node->sock);
    linq_free(node);
}
