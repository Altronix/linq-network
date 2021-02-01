#include "mock_zmq.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_msg(void** context_p)
{
    zmq_msg_t a, b, c;
    zmq_msg_init_size(&a, 1);
    zmq_msg_init_size(&b, 1);
    zmq_msg_init_size(&c, 1);
    (*(char*)zmq_msg_data(&a)) = 'a';
    (*(char*)zmq_msg_data(&b)) = 'b';
    (*(char*)zmq_msg_data(&c)) = 'c';

    zmq_msg_send(&a, NULL, ZMQ_SNDMORE);
    zmq_msg_send(&b, NULL, ZMQ_SNDMORE);
    zmq_msg_send(&c, NULL, 0);

    zmq_msg_close(&a);
    zmq_msg_close(&b);
    zmq_msg_close(&c);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_mock_msg) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
