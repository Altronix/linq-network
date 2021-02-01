#include "mock_zmq.h"
#include "zmq.h"

static uint32_t incoming = 0;

void
zmq_spy_poll_set_incoming(uint32_t val)
{
    incoming = val;
}

void
zmq_spy_poll_reset()
{
    incoming = 0;
}

void*
__wrap_zmq_ctx_new()
{}

int
__wrap_zmq_ctx_destroy(void* ctx)
{}

int
__wrap_zmq_bind(void* socket, const char* ep)
{}

int
__wrap_zmq_connect(void* socket, const char* ep)
{}

int
__wrap_zmq_close(void* socket)
{}

int
__wrap_zmq_poll(zmq_pollitem_t* items_, int nitems_, long timeout_)
{

    ((void)timeout_);
    for (int i = 0; i < nitems_ && i < 32; i++) {
        if (incoming & (0x01 << i)) {
            items_[i].revents |= ZMQ_POLLIN;
        } else {
            items_[i].revents &= (~(ZMQ_POLLIN));
        }
    }
    return 0;
}

int
__wrap_zmq_send(void* socket, void* buf, size_t len, int flags)
{}

int
__wrap_zmq_recv(void* socket, void* buf, size_t len, int flags)
{}
