// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MOCK_ZSOCK_H_
#define MOCK_ZSOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <czmq.h>

int czmq_spy_push_incoming_mesg(zmsg_t** msg_p);

#ifdef __cplusplus
}
#endif
#endif
