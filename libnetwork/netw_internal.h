// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Common lib
#include "common.h"

// project includes
#include "libcommon/device.h"
#include "node.h"
#include "zmtp.h"
#if BUILD_LINQD
#include "database.h"
#include "http.h"
#endif
#ifdef BUILD_USBH
#include "usbh.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // Main class
    typedef struct netw_s
    {
        const netw_callbacks* callbacks;
        void* context;
        bool shutdown;
        device_map_s* devices;
        node_map_s* nodes;
        zmtp_s zmtp;
#ifdef BUILD_USBH
        usbh_s usb;
#endif
#ifdef BUILD_LINQD
        http_s http;
        database_s database;
#endif
        char uuid[UUID_LEN * 2 + 1];
    } netw_s;

#ifdef __cplusplus
}
#endif

#endif /* LINQ_INTERNAL_H_ */
