#ifndef MOCK_ZMQ_H_
#define MOCK_ZMQ_H_

#include "stdint.h"
#include "zmq.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct mock_zmq_msg_s
    {
        zmq_msg_t* msg;
        int flags;
    } mock_zmq_msg_s;

    void zmq_spy_init();
    void zmq_spy_free();
    void zmq_spy_poll_set_ready(uint32_t val);
    mock_zmq_msg_s* zmq_spy_mesg_at_outgoing(int);
    mock_zmq_msg_s* zmq_spy_mesg_at_incoming(int);
    void zmq_spy_mesg_close_outgoing(int at);

#ifdef __cplusplus
}
#endif
#endif
