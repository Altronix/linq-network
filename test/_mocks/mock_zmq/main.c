#include "mock_zmq.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>

#include <cmocka.h>

static void
assert_msg_equal(zmq_msg_t* msg, int flags, void* data, uint32_t l)
{
    void* src = zmq_msg_data(msg);
    assert_int_equal(zmq_msg_size(msg), l);
    assert_int_equal(zmq_msg_more(msg), flags ? 1 : 0);
    assert_memory_equal(src, data, l);
}

static void
assert_recv_msg_equal(int flags, void* data, uint32_t l)
{
    int sz;
    zmq_msg_t incoming;
    zmq_msg_init(&incoming);
    sz = zmq_msg_recv(&incoming, NULL, 0);
    assert_int_equal(sz, zmq_msg_size(&incoming));
    assert_msg_equal(&incoming, flags, data, l);
    zmq_msg_close(&incoming);
}

static void
test_mock_send(void** context_p)
{
    zmq_spy_init();

    zmq_msg_t a, b, c, outgoing;
    zmq_msg_init_size(&a, 3);
    zmq_msg_init_size(&b, 3);
    zmq_msg_init_size(&c, 3);
    memcpy(zmq_msg_data(&a), "aaa", 3);
    memcpy(zmq_msg_data(&b), "bbb", 3);
    memcpy(zmq_msg_data(&c), "ccc", 3);

    zmq_msg_send(&a, NULL, ZMQ_SNDMORE);
    zmq_msg_send(&b, NULL, ZMQ_SNDMORE);
    zmq_msg_send(&c, NULL, 0);

    assert_msg_equal(zmq_spy_mesg_at_outgoing(0)->msg, ZMQ_SNDMORE, "aaa", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(1)->msg, ZMQ_SNDMORE, "bbb", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(2)->msg, 0, "ccc", 3);

    zmq_msg_close(&a);
    zmq_msg_close(&b);
    zmq_msg_close(&c);

    zmq_spy_free();
}

static void
test_mock_send_large(void** context_p)
{
    char b[4096];
    zmq_spy_init();
    zmq_msg_t a;
    zmq_msg_init_size(&a, sizeof(b));
    memset(b, 'A', sizeof(b));
    memcpy(zmq_msg_data(&a), b, sizeof(b));
    zmq_msg_send(&a, NULL, 0);
    mock_zmq_msg_s* m = zmq_spy_mesg_at_outgoing(0);
    assert_int_equal(m->flags, 0);
    assert_int_equal(zmq_msg_size(m->msg), sizeof(b));
    assert_memory_equal(zmq_msg_data(m->msg), b, 4096);
    zmq_spy_free();
}

static void
test_mock_recv(void** context_p)
{
    zmq_spy_init();
    zmq_msg_t a, b, c, x, y, z, incoming;
    zmq_msg_init_size(&a, 3);
    zmq_msg_init_size(&b, 3);
    zmq_msg_init_size(&c, 3);
    zmq_msg_init_size(&x, 3);
    zmq_msg_init_size(&y, 3);
    zmq_msg_init_size(&z, 3);
    memcpy(zmq_msg_data(&a), "aaa", 3);
    memcpy(zmq_msg_data(&b), "bbb", 3);
    memcpy(zmq_msg_data(&c), "ccc", 3);
    memcpy(zmq_msg_data(&x), "xxx", 3);
    memcpy(zmq_msg_data(&y), "yyy", 3);
    memcpy(zmq_msg_data(&x), "zzz", 3);

    zmq_spy_msg_push_incoming(&a, ZMQ_SNDMORE);
    zmq_spy_msg_push_incoming(&b, ZMQ_SNDMORE);
    zmq_spy_msg_push_incoming(&c, 0);

    zmq_spy_msg_push_incoming(&x, ZMQ_SNDMORE);
    zmq_spy_msg_push_incoming(&y, ZMQ_SNDMORE);
    zmq_spy_msg_push_incoming(&z, 0);

    assert_recv_msg_equal(ZMQ_SNDMORE, "aaa", 3);
    // assert_recv_msg_equal(ZMQ_SNDMORE, "bbb", 3);
    // assert_recv_msg_equal(0, "ccc", 3);
    // assert_recv_msg_equal(ZMQ_SNDMORE, "xxx", 3);
    // assert_recv_msg_equal(ZMQ_SNDMORE, "yyy", 3);
    // assert_recv_msg_equal(0, "zzz", 3);

    zmq_spy_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_mock_send),
                                        cmocka_unit_test(test_mock_send_large),
                                        cmocka_unit_test(test_mock_recv) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
