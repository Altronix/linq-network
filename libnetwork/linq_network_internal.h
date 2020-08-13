// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Dependencies includes
#include "czmq.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

// project includes
#include "linq_network.h"

#if WITH_SQLITE
#include "database/database.h"
#include "http.h"
#endif

// [router, version, type, serial]
// [router, version, 0x00, serial, type, siteId]         = HEARTBEAT
// [router, version, 0x01, serial, path [, data] ]       = REQUEST
// [router, version, 0x02, serial, error, data]          = RESPONSE
// [router, version, 0x03, serial, type, alert, mail]    = ALERT
// [router, version, 0x04, serial, ...]                  = HELLO

#define B64_LEN(x) ((4 / 3) * (x + 2))

#define RID_LEN 256
#define B64_RID_LEN (B64_LEN(RID_LEN) + 1)
#define SID_LEN LINQ_SID_LEN
#define TID_LEN LINQ_TID_LEN
#define SITE_LEN 64
#define JSON_LEN 1024

#define FRAME_VER_0 ('\x0')
#define FRAME_TYP_HEARTBEAT ('\x0')
#define FRAME_TYP_REQUEST ('\x1')
#define FRAME_TYP_RESPONSE ('\x2')
#define FRAME_TYP_ALERT ('\x3')
#define FRAME_TYP_HELLO ('\x4')

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

    typedef struct
    {
        uint8_t id[256];
        uint32_t sz;
    } router_s;

    typedef struct
    {
        zsock_t* sock;
        router_s* router;
    } linq_network_socket_s;

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

#ifdef WITH_SQLITE
    database_s* linq_network_database(linq_network_s* l);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LINQ_INTERNAL_H_ */
