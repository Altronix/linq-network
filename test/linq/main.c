#include "altronix/linq.h"
#include "helpers.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static e_linq_error expect_error = e_linq_ok;
static const char* expect_what = "";
static const char* expect_serial = "";

static void
linq_on_error_fn(
    void* pass,
    e_linq_error e,
    const char* what,
    const char* serial)
{
    assert_int_equal(e, expect_error);
    assert_string_equal(what, expect_what);
    assert_string_equal(serial, expect_serial);
    *((bool*)pass) = true;
}

linq_callbacks callbacks = { .err = linq_on_error_fn };

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
    bool pass = false;
    linq* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    expect_error = e_linq_protocol;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_push_incoming(true);

    linq_poll(l);

    assert_true(pass);

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
