// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_H_
#define NODE_H_

#include "common.h"
#include "containers.h"
#include "czmq.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these
#include "netw_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct node_zmtp_s node_zmtp_s;
    MAP_INIT_H(node, node_zmtp_s);

    node_zmtp_s* node_create(
        zsock_t*,
        const uint8_t* router,
        uint32_t router_sz,
        const char*);
    void node_destroy(node_zmtp_s**);
    void node_update_router(node_zmtp_s*, const uint8_t*, uint32_t);
    const char* node_serial(node_zmtp_s*);
    void node_send_hello(node_zmtp_s* node);
    void node_send_frames(node_zmtp_s* node, uint32_t n, zframe_t** frames);
    void node_send_frames_n(node_zmtp_s* node, uint32_t n, ...);
    uint32_t node_map_foreach_remove_if(
        node_map_s* hash,
        bool (*remove)(node_s*, void*),
        void* ctx);
#ifdef __cplusplus
}
#endif
#endif /* NODE_H_ */
