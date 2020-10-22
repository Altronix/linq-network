// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZMTP_H
#define ZMTP_H

#include "containers.h"
#include "libcommon/device.h"
#include "node.h"

// [router, version, type, serial]
// [router, version, 0x00, serial, type, siteId]         = HEARTBEAT
// [router, version, 0x01, serial, path [, data] ]       = REQUEST
// [router, version, 0x02, serial, error, data]          = RESPONSE
// [router, version, 0x03, serial, type, alert, mail]    = ALERT
// [router, version, 0x04, serial, ...]                  = HELLO

#ifndef LINQ_NETW_MAX_RETRY
#define LINQ_NETW_MAX_RETRY 5
#endif

#ifndef LINQ_NETW_RETRY_TIMEOUT
#define LINQ_NETW_RETRY_TIMEOUT 35000
#endif

#define FRAME_VER_0 ('\x0')
#define FRAME_TYP_HEARTBEAT ('\x0')
#define FRAME_TYP_REQUEST ('\x1')
#define FRAME_TYP_RESPONSE ('\x2')
#define FRAME_TYP_ALERT ('\x3')
#define FRAME_TYP_HELLO ('\x4')
#define FRAME_TYPE(x) ((x) & (~(0x80)))
#define FRAME_IS_BROADCAST(x) (((x) & ((0x80))) == 0x80)
#define FRAME_SET_BROADCAST(x) ((x) |= (0x80))

// Basic packet index's
#define FRAME_MAX 7
#define FRAME_RID_IDX 0
#define FRAME_VER_IDX 1
#define FRAME_TYP_IDX 2
#define FRAME_SID_IDX 3

// Heartbeat IDX
#define FRAME_HB_TID_IDX 4
#define FRAME_HB_SITE_IDX 5

// Alert IDX
#define FRAME_ALERT_TID_IDX 4
#define FRAME_ALERT_DAT_IDX 5
#define FRAME_ALERT_DST_IDX 6

// Request IDX
#define FRAME_REQ_PATH_IDX 4
#define FRAME_REQ_DATA_IDX 5

// Response IDX
#define FRAME_RES_ERR_IDX 4
#define FRAME_RES_DAT_IDX 5

// Maximum sockets
#define MAX_CONNECTIONS 10

#define LINQ_ERROR_SOCKET 0xFFFFFFFF

#define B64_LEN(x) ((4 / 3) * (x + 2))

#define RID_LEN 256
#define B64_RID_LEN (B64_LEN(RID_LEN) + 1)
#define SID_LEN LINQ_SID_LEN
#define TID_LEN LINQ_TID_LEN
#define SITE_LEN 64
#define JSON_LEN 8192

#define ATX_NET_SOCKET(socket) ((0x00FF) & socket)
#define ATX_NET_SOCKET_TYPE(socket) ((socket & 0xFF00) >> 0x08)
#define ATX_NET_SOCKET_TYPE_ROUTER (0x01)
#define ATX_NET_SOCKET_TYPE_DEALER (0x02)
#define ATX_NET_SOCKET_TYPE_HTTP (0x03)
#define ATX_NET_SOCKET_TYPE_IS_ROUTER(socket)                                  \
    (ATX_NET_SOCKET_TYPE(socket) == ATX_NET_SOCKET_TYPE_ROUTER)
#define ATX_NET_SOCKET_TYPE_IS_DEALER(socket)                                  \
    (ATX_NET_SOCKET_TYPE(socket) == ATX_NET_SOCKET_TYPE_DEALER)
#define ATX_NET_SOCKET_TYPE_IS_HTTP(socket)                                    \
    (ATX_NET_SOCKET_TYPE(socket) == ATX_NET_SOCKET_TYPE_HTTP)

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
    LINQ_EXPORT bool sys_running();
#ifdef __cplusplus
}
#endif
#endif
