#include "mock_zmq.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>

#include <cmocka.h>

static void
assert_msg_equal(mock_zmq_msg_s* msg, int flags, void* data, uint32_t l)
{
    void* src = zmq_msg_data(msg->msg);
    assert_int_equal(zmq_msg_size(msg->msg), l);
    assert_int_equal(msg->flags, flags);
    assert_memory_equal(src, data, l);
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

    assert_msg_equal(zmq_spy_mesg_at_outgoing(0), ZMQ_SNDMORE, "aaa", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(1), ZMQ_SNDMORE, "bbb", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(2), 0, "ccc", 3);

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
