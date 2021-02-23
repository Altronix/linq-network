#include "mock_zmq.h"
#include "containers.h"
#include "zmq.h"
extern int __real_zmq_msg_close(zmq_msg_t* msg);

VEC_INIT_W_HEADER(msg, mock_zmq_msg_s);
static uint32_t ready = 0;
msg_vec_s outgoing;
msg_vec_s incoming;

static int
has_more(mock_zmq_msg_s* mock)
{
    return (mock->flags & ZMQ_SNDMORE || mock->flags & ZMQ_RCVMORE) ? 1 : 0;
}

static mock_zmq_msg_s*
find_msg_in_vec(msg_vec_s* vec, zmq_msg_t* m)
{
    for (int i = 0; i < msg_vec_size(vec); i++) {
        mock_zmq_msg_s* mock = msg_vec_at(vec, i);
        if (mock->msg == m || mock->recvd == m) return msg_vec_at(vec, i);
    }
    return NULL;
}

static mock_zmq_msg_s*
find_msg(zmq_msg_t* m)
{
    mock_zmq_msg_s* msg = find_msg_in_vec(&incoming, m);
    return msg ? msg : find_msg_in_vec(&outgoing, m);
}

void
zmq_spy_init()
{
    ready = 0;
    msg_vec_init(&outgoing);
    msg_vec_init(&incoming);
}

void
zmq_spy_free()
{
    ready = 0;
    for (int i = 0; i < msg_vec_size(&outgoing); i++) {
        zmq_spy_mesg_close_outgoing(i);
    }
    for (int i = 0; i < msg_vec_size(&incoming); i++) {
        zmq_spy_mesg_close_incoming(i);
    }
    msg_vec_free(&outgoing);
    msg_vec_free(&incoming);
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
    mock_zmq_msg_s* m = msg_vec_at(&outgoing, at);
    if (m && (!m->closed)) zmq_msg_close(msg_vec_at(&outgoing, at)->msg);
}

void
zmq_spy_mesg_close_incoming(int at)
{
    mock_zmq_msg_s* m = msg_vec_at(&incoming, at);
    if (m && (!m->closed)) zmq_msg_close(msg_vec_at(&incoming, at)->msg);
}

mock_zmq_msg_s*
zmq_spy_msg_push_incoming(zmq_msg_t* msg, int f)
{
    mock_zmq_msg_s mock = { .msg = msg, .flags = f, .closed = 0, .recvd = 0 };
    msg_vec_push(&incoming, mock);
    return msg_vec_last(&incoming);
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
__wrap_zmq_msg_send(zmq_msg_t* msg, void* socket, int f)
{
    mock_zmq_msg_s mock = { .msg = msg, .flags = f, .closed = 0, .recvd = 0 };
    msg_vec_push(&outgoing, mock);
    return zmq_msg_size(msg);
}

int
__wrap_zmq_msg_recv(zmq_msg_t* msg, void* socket, int flags)
{
    for (int i = 0; i < msg_vec_size(&incoming); i++) {
        mock_zmq_msg_s* mock = msg_vec_at(&incoming, i);
        if (!mock->recvd) {
            mock->recvd = msg;
            *msg = *mock->msg;
            return zmq_msg_size(msg);
        }
    }
    return -1;
}

int
__wrap_zmq_msg_close(zmq_msg_t* msg)
{
    mock_zmq_msg_s* mock = find_msg(msg);
    if (mock) {
        assert(!mock->closed);
        mock->closed = 1;
    }
    int ret = __real_zmq_msg_close(msg);
    assert(ret == 0);
    return ret;
}

int
__wrap_zmq_msg_more(zmq_msg_t* msg)
{
    mock_zmq_msg_s* mock = find_msg(msg);
    assert(mock);
    return has_more(mock);
}

int
__wrap_zmq_getsockopt(void* s_, int option_, void* optval_, size_t* optvallen_)
{
    if (option_ == ZMQ_RCVMORE) {
        if (has_more(msg_vec_last(&incoming))) {
            *((int*)optval_) = 1;
        } else {
            *((int*)optval_) = 0;
        }
        return 0;
    }
    return -1;
}
