// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MOCK_ZMSG_H_
#define MOCK_ZMSG_H_

#include <czmq.h>
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

#ifdef __cplusplus
extern "C"
{
#endif

    void czmq_spy_mesg_push_incoming(zmsg_t** msg_p);
    void czmq_spy_mesg_push_outgoing(zmsg_t** msg_p);
    void czmq_spy_mesg_reset(void);
    zmsg_t* czmq_spy_mesg_pop_outgoing();
    zmsg_t* czmq_spy_mesg_pop_incoming();
    void czmq_spy_mesg_flush_outgoing();
    void czmq_spy_mesg_flush_incoming();

#ifdef __cplusplus
}
#endif
#endif
