#include "nodes.h"
#include "node.h"

KHASH_MAP_INIT_STR(node, node_s*);

typedef struct nodes_s
{
    kh_node_t* h;
} nodes_s;

nodes_s*
nodes_create()
{
    nodes_s* d = linq_malloc(sizeof(nodes_s));
    if (d) { d->h = kh_init(node); }
    return d;
}

void
nodes_destroy(nodes_s** d_p)
{
    nodes_s* dmap = *d_p;
    *d_p = NULL;
    for (khint_t k = kh_begin(dmap->h); k != kh_end(dmap->h); ++k) {
        if (kh_exist(dmap->h, k)) {
            node_s* d = kh_val(dmap->h, k);
            node_destroy(&d);
        }
    }
    kh_destroy(node, dmap->h);
    linq_free(dmap);
}

node_s**
nodes_add(nodes_s* nodes, node_s** node_p)
{
    int ret = 0;
    node_s* node = *node_p;
    *node_p = NULL;
    khiter_t k = kh_put(node, nodes->h, node_serial(node), &ret);
    linq_assert(ret == 1); // If double insert we crash
    kh_val(nodes->h, k) = node;
    return &kh_val(nodes->h, k);
}
uint32_t
nodes_remove(nodes_s* dmap, const char* serial)
{
    khiter_t k;
    node_s* d;
    uint32_t count = 0;
    if (!((k = kh_get(node, dmap->h, serial)) == kh_end(dmap->h))) {
        d = kh_val(dmap->h, k);
        kh_del(node, dmap->h, k);
        node_destroy(&d);
        count = 1;
    }
    return count;
}

node_s**
nodes_get(nodes_s* dmap, const char* serial)
{
    khiter_t k;
    return ((k = kh_get(node, dmap->h, serial)) == kh_end(dmap->h))
               ? NULL
               : &kh_val(dmap->h, k);
}

uint32_t
nodes_size(nodes_s* map)
{
    return kh_size(map->h);
}
