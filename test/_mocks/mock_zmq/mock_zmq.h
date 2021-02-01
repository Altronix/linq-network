#ifndef MOCK_ZMQ_H_
#define MOCK_ZMQ_H_

#include "stdint.h"
#include "zmq.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ZMQ_MSG(x) ((zmq_msg_t*)x)
#define MOCK_ZMQ_MSG(x) ((mock_zmq_msg_s*)x)

    typedef struct mock_zmq_msg_s
    {
        zmq_msg_t msg;
        int flags;
    } mock_zmq_msg_s;

    void zmq_spy_init();
    void zmq_spy_free();
    void zmq_spy_poll_set_ready(uint32_t val);
    mock_zmq_msg_s zmq_spy_mesg_pop_outgoing();
    mock_zmq_msg_s zmq_spy_mesg_pop_incoming();
    void zmq_spy_mesg_flush_outgoing();
    void zmq_spy_mesg_flush_incoming();

#ifdef __cplusplus
}
#endif
#endif
