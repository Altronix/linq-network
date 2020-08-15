// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZMTP_H
#define ZMTP_H

#include "containers.h"
#include "device.h"
#include "netw_internal.h"
#include "node.h"

#ifndef LINQ_NETW_MAX_RETRY
#define LINQ_NETW_MAX_RETRY 5
#endif

#ifndef LINQ_NETW_RETRY_TIMEOUT
#define LINQ_NETW_RETRY_TIMEOUT 500
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // helpful struct for maintaining frames
    typedef struct
    {
        uint32_t n;
        zframe_t** frames;
    } frames_s;

    typedef unsigned char uchar;

    extern char g_frame_ver_0;
    extern char g_frame_typ_heartbeat;
    extern char g_frame_typ_request;
    extern char g_frame_typ_response;
    extern char g_frame_typ_alert;
    extern char g_frame_typ_hello;

    typedef struct
    {
        uint8_t id[256];
        uint32_t sz;
    } router_s;

    typedef struct
    {
        node_s base;
        zsock_t* sock;
        router_s* router;
    } netw_socket_s;

    MAP_INIT_H(socket, zsock_t);
    typedef struct zmtp_s
    {
        void* context;
        socket_map_s* routers;
        socket_map_s* dealers;
        device_map_s** devices_p;
        node_map_s** nodes_p;
        bool shutdown;
        const netw_callbacks* callbacks;
    } zmtp_s;

    void zmtp_init(
        zmtp_s* zmtp,
        device_map_s** devices_p,
        node_map_s** nodes_p,
        const netw_callbacks* callbacks,
        void* ctx);
    void zmtp_deinit(zmtp_s* zmtp);
    netw_socket zmtp_listen(zmtp_s* zmtp, const char* ep);
    netw_socket zmtp_connect(zmtp_s* l, const char* ep);
    E_LINQ_ERROR zmtp_close_router(zmtp_s* zmtp, netw_socket sock);
    E_LINQ_ERROR zmtp_close_dealer(zmtp_s* zmtp, netw_socket sock);
    E_LINQ_ERROR
    zmtp_poll(zmtp_s* self, int32_t ms);
    E_LINQ_ERROR
    zmtp_send(
        const zmtp_s* zmtp,
        const char* sid,
        const char* meth,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn,
        void* ctx);
#ifdef __cplusplus
}
#endif
#endif
