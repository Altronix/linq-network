#ifndef NODE_H_
#define NODE_H_

#include "czmq.h"
#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct node_s node_s;

    node_s* node_connect(const char* ep);
#ifdef __cplusplus
}
#endif

#endif /* NODE_H_ */
