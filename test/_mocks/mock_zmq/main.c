#include "mock_zmq.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>

#include <cmocka.h>

static void
assert_msg_equal(mock_zmq_msg_s* msg, int more, void* data, uint32_t l)
{
    void* src = zmq_msg_data(ZMQ_MSG(msg));
    assert_int_equal(zmq_msg_size(ZMQ_MSG(msg)), l);
    assert_int_equal(zmq_msg_more(ZMQ_MSG(msg)), more);
    assert_memory_equal(src, data, l);
}

static void
test_mock_send(void** context_p)
{
    zmq_spy_init();

    mock_zmq_msg_s a, b, c, outgoing;
    zmq_msg_init_size(ZMQ_MSG(&a), 3);
    zmq_msg_init_size(ZMQ_MSG(&b), 3);
    zmq_msg_init_size(ZMQ_MSG(&c), 3);
    memcpy(zmq_msg_data(ZMQ_MSG(&a)), "aaa", 3);
    memcpy(zmq_msg_data(ZMQ_MSG(&b)), "bbb", 3);
    memcpy(zmq_msg_data(ZMQ_MSG(&c)), "ccc", 3);

    zmq_msg_send(ZMQ_MSG(&a), NULL, ZMQ_SNDMORE);
    zmq_msg_send(ZMQ_MSG(&b), NULL, ZMQ_SNDMORE);
    zmq_msg_send(ZMQ_MSG(&c), NULL, 0);

    assert_msg_equal(zmq_spy_mesg_at_outgoing(0), 1, "aaa", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(1), 1, "bbb", 3);
    assert_msg_equal(zmq_spy_mesg_at_outgoing(2), 0, "ccc", 3);

    zmq_msg_close(ZMQ_MSG(&a));
    zmq_msg_close(ZMQ_MSG(&b));
    zmq_msg_close(ZMQ_MSG(&c));

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
                                        cmocka_unit_test(test_mock_recv) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
