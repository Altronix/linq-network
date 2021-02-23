#include "more.h"

#include "msg.hpp"

void
more_set(zmq_msg_t* msg)
{
    ((zmq::msg_t*)msg)->set_flags(zmq::msg_t::more);
}
