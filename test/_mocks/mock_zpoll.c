#include "mock_zpoll.h"
#include "zmq.h"

static bool incoming = false;

void
czmq_spy_poll_push_incoming(bool i)
{
    incoming = i;
}

void
czmq_spy_poll_reset()
{
    incoming = false;
}

int
__wrap_zmq_poll(zmq_pollitem_t* items_, int nitems_, long timeout_)
{
    ((void)nitems_);
    ((void)timeout_);
    if (incoming) {
        items_->revents |= ZMQ_POLLIN;
    } else {
        items_->revents &= (~(ZMQ_POLLIN));
    }
    return 0;
}
