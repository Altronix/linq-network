#ifndef NODES_H_
#define NODES_H_

#include "czmq.h"

#include "linq_internal.h"
#include "node.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct nodes_s nodes_s;

    nodes_s* nodes_create();
    void nodes_destroy(nodes_s**);
    node_s** nodes_insert(nodes_s* nodes, const char* id, node_s** node_p);
    uint32_t nodes_size(nodes_s*);
    node_s** nodes_get(nodes_s* nodes, const char* id);
    uint32_t nodes_remove(nodes_s* nodes, const char* id);

#ifdef __cplusplus
}
#endif

#endif /* NODES_H_ */
