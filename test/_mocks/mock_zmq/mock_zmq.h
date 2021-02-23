#ifndef MOCK_ZMQ_H_
#define MOCK_ZMQ_H_

#include "more.h"
#include "stdint.h"
#include "zmq.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct mock_zmq_send_msg_s
    {
        zmq_msg_t* msg;
        int closed, recvd;
    } mock_zmq_msg_s;

    void zmq_spy_init();
    void zmq_spy_free();
    void zmq_spy_poll_set_ready(uint32_t val);
    mock_zmq_msg_s* zmq_spy_mesg_at_outgoing(int);
    mock_zmq_msg_s* zmq_spy_msg_push_incoming(zmq_msg_t* m, int);
    void zmq_spy_mesg_close_outgoing(int at);
    void zmq_spy_mesg_close_incoming(int at);

#ifdef __cplusplus
}
#endif
#endif
