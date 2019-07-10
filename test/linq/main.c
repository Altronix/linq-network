#include "altronix/linq.h"
#include "helpers.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_linq_create(void** context_p)
{
    ((void)context_p);
    linq* l = linq_create(NULL, NULL);
    assert_non_null(l);
    linq_destroy(&l);
    assert_null(l);
}

static void
test_linq_receive_protocol_error_short(void** context_p)
{
    ((void)context_p);
    linq* l = linq_create(NULL, NULL);
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_push_incoming(true);

    linq_poll(l);

    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
    linq_destroy(&l);
}

static void
test_linq_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_linq_create),
        cmocka_unit_test(test_linq_receive_heartbeat_ok),
        cmocka_unit_test(test_linq_receive_protocol_error_short)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
