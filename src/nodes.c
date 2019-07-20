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

node_s**
nodes_insert(nodes_s* nodes, const char* id, node_s** node_p)
{
    int ret;
    node_s* n = *node_p;
    *node_p = NULL;
    khiter_t k = kh_put(node, nodes->h, id, &ret);
    linq_assert(ret == 1);
    kh_val(nodes->h, k) = n;
    return &kh_val(nodes->h, k);
}

node_s**
nodes_get(nodes_s* nodes, const char* id)
{
    khiter_t k;
    return ((k = kh_get(node, nodes->h, id)) == kh_end(nodes->h))
               ? NULL
               : &kh_val(nodes->h, k);
}

uint32_t
nodes_remove(nodes_s* nodes, const char* id)
{
    khiter_t k;
    node_s* n;
    uint32_t count = 0;
    if (!((k = kh_get(node, nodes->h, id)) == kh_end(nodes->h))) {
        n = kh_val(nodes->h, k);
        kh_del(node, nodes->h, k);
        node_destroy(&n);
        count = 1;
    }
    return count;
}

uint32_t
nodes_size(nodes_s* nodes)
{
    return kh_size(nodes->h);
}
