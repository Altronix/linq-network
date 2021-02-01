#include "mock_zmq.h"
#include "containers.h"
#include "zmq.h"

VEC_INIT_W_HEADER(msg, mock_zmq_msg_s*);
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
    msg_vec_free(&incoming);
    msg_vec_free(&outgoing);
}

void
zmq_spy_poll_set_ready(uint32_t val)
{
    ready = val;
}

mock_zmq_msg_s*
zmq_spy_mesg_pop_outgoing()
{
    return msg_vec_pop(&outgoing);
}

mock_zmq_msg_s*
zmq_spy_mesg_pop_incoming()
{
    return msg_vec_pop(&incoming);
}

void
zmq_spy_mesg_flush_outgoing()
{
    while (msg_vec_size(&outgoing)) linq_network_free(msg_vec_pop(&outgoing));
}

void
zmq_spy_mesg_flush_incoming()
{
    while (msg_vec_size(&incoming)) linq_network_free(msg_vec_pop(&incoming));
}

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
    msg_vec_push(&outgoing, &msg);
    return zmq_msg_size(msg);
}

int
__wrap_zmq_msg_recv(zmq_msg_t* msg, void* socket, int flags)
{
    msg = msg_vec_pop(&incoming);
    return zmq_msg_size(msg);
}
