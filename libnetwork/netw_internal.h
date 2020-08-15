// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Common lib
#include "common.h"

// Dependencies includes
#include "czmq.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

// project includes
#include "netw.h"

#if BUILD_LINQD
#include "database.h"
#include "http.h"
#endif

// [router, version, type, serial]
// [router, version, 0x00, serial, type, siteId]         = HEARTBEAT
// [router, version, 0x01, serial, path [, data] ]       = REQUEST
// [router, version, 0x02, serial, error, data]          = RESPONSE
// [router, version, 0x03, serial, type, alert, mail]    = ALERT
// [router, version, 0x04, serial, ...]                  = HELLO

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* LINQ_INTERNAL_H_ */
