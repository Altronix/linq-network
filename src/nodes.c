#include "nodes.h"
#include "klib/khash.h"
#include "node.h"

KHASH_MAP_INIT_STR(node, node_s*);

typedef struct nodes_s
{
    kh_node_t* h;
} nodes_s;

nodes_s*
nodes_create()
{
    nodes_s* nodes = linq_malloc(sizeof(nodes_s));
    if (nodes) nodes->h = kh_init(node);
    return nodes;
}

void
nodes_destroy(nodes_s** nodes_p)
{
    nodes_s* nodes = *nodes_p;
    *nodes_p = NULL;
    for (khint_t k = kh_begin(nodes->h); k != kh_end(nodes->h); ++k) {
        if (kh_exist(nodes->h, k)) {
            node_s* n = kh_val(nodes->h, k);
            node_destroy(&n);
        }
    }
    kh_destroy(node, nodes->h);
    linq_free(nodes);
}
