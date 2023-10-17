#include "message.h"

#include "msg.hpp"

void
message_more_set(zmq_msg_t* msg)
{
    ((zmq::msg_t*)msg)->set_flags(zmq::msg_t::more);
}

int
message_valid(zmq_msg_t* msg)
{
    return ((zmq::msg_t*)msg)->check();
}
