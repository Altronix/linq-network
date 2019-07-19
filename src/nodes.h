#ifndef NODES_H_
#define NODES_H_

#include "czmq.h"
#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct nodes_s nodes_s;

    nodes_s* nodes_create();
    void nodes_destroy(nodes_s**);

#ifdef __cplusplus
}
#endif

#endif /* NODES_H_ */
