#include "mock_zmq.h"
#include "containers.h"
#include "zmq.h"

VEC_INIT_W_HEADER(msg, mock_zmq_msg_s);
static uint32_t ready = 0;
msg_vec_s incoming;
msg_vec_s outgoing;

void
zmq_spy_init()
{
    ready = 0;
    msg_vec_init(&incoming);
    msg_vec_init(&outgoing);
}

void
zmq_spy_free()
{
    ready = 0;
    for (int i = 0; i < msg_vec_size(&outgoing); i++) {
        zmq_spy_mesg_close_outgoing(i);
    }
    msg_vec_free(&incoming);
    msg_vec_free(&outgoing);
}

void
zmq_spy_poll_set_ready(uint32_t val)
{
    ready = val;
}

mock_zmq_msg_s*
zmq_spy_mesg_at_outgoing(int at)
{
    return msg_vec_at(&outgoing, at);
}

void
zmq_spy_mesg_close_outgoing(int at)
{
    zmq_msg_close(msg_vec_at(&outgoing, at)->msg);
}

mock_zmq_msg_s*
zmq_spy_mesg_at_incoming(int at)
{
    return msg_vec_at(&incoming, at);
}

int
zmq_spy_msg_push_incoming()
{}

void*
__wrap_zmq_ctx_new()
{
    static int ret = 1;
    return &ret;
}

int
__wrap_zmq_ctx_destroy(void* ctx)
{
    ((void)ctx);
    return 0;
}

int
__wrap_zmq_bind(void* socket, const char* ep)
{
    return 0;
}

int
__wrap_zmq_connect(void* socket, const char* ep)
{
    return 0;
}

int
__wrap_zmq_close(void* socket)
{
    return 0;
}

int
__wrap_zmq_poll(zmq_pollitem_t* items_, int nitems_, long timeout_)
{

    ((void)timeout_);
    for (int i = 0; i < nitems_ && i < 32; i++) {
        if (ready & (0x01 << i)) {
            items_[i].revents |= ZMQ_POLLIN;
        } else {
            items_[i].revents &= (~(ZMQ_POLLIN));
        }
    }
    return 0;
}

int
__wrap_zmq_msg_send(zmq_msg_t* msg, void* socket, int flags)
{
    mock_zmq_msg_s mock = { .msg = msg, .flags = flags };
    msg_vec_push(&outgoing, mock);
    return zmq_msg_size(msg);
}

int
__wrap_zmq_msg_recv(zmq_msg_t* msg, void* socket, int flags)
{
    msg = msg_vec_last(&incoming)->msg;
    int sz = zmq_msg_size(msg);
    return sz;
}

int
__wrap_zmq_msg_more(zmq_msg_t* msg)
{
    return -1;
}

int
__wrap_zmq_getsockopt(void* s_, int option_, void* optval_, size_t* optvallen_)
{
    if (option_ == ZMQ_RCVMORE) {
        if (msg_vec_last(&incoming)->flags & ZMQ_SNDMORE) {
            *((int*)optval_) = 1;
        } else {
            *((int*)optval_) = 0;
        }
        return 0;
    }
    return -1;
}
