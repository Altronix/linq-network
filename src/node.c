#include "node.h"

typedef struct node_s
{
    zsock_t* sock;
} node_s;

// Connect to a remote rep socket
node_s*
node_connect(const char* ep)
{}
