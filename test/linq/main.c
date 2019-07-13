#include "altronix/linq.h"
#include "device.h"
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static E_LINQ_ERROR expect_error = LINQ_ERROR_OK;
static const char* empty = "";
static const char* expect_what = "";
static const char* expect_serial = "";

static void
test_reset()
{
    expect_error = LINQ_ERROR_OK;
    expect_what = empty;
    expect_serial = empty;
    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
}

static void
linq_on_error_fn(
    void* pass,
    E_LINQ_ERROR e,
    const char* what,
    const char* serial)
{
    assert_int_equal(e, expect_error);
    assert_string_equal(what, expect_what);
    assert_string_equal(serial, expect_serial);
    *((bool*)pass) = true;
}

static void
linq_on_heartbeat_fn(void* pass, const char* serial, device_s** d)
{
    assert_string_equal(serial, expect_serial);
    assert_string_equal(device_serial(*d), expect_serial);
    *((bool*)pass) = true;
}

static void
linq_on_alert_fn(void* pass, linq_alert* alert, linq_email* email, device_s** d)
{
    assert_string_equal(device_serial(*d), expect_serial);
    assert_string_equal(alert->who, "TestUser");
    assert_string_equal(alert->what, "TestAlert");
    assert_string_equal(alert->where, "Altronix Site ID");
    assert_string_equal(alert->when, "100");
    assert_string_equal(alert->mesg, "Test Alert Message");
    assert_string_equal(email->to0, "mail0@gmail.com");
    assert_string_equal(email->to1, "mail1@gmail.com");
    assert_string_equal(email->to2, "mail2@gmail.com");
    assert_string_equal(email->to3, "mail3@gmail.com");
    assert_string_equal(email->to4, "mail4@gmail.com");
    *((bool*)pass) = true;
}

linq_callbacks callbacks = { .err = linq_on_error_fn,
                             .hb = linq_on_heartbeat_fn,
                             .alert = linq_on_alert_fn };

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

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_push_incoming(true);

    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_protocol_error_serial(void** context_p)
{
    ((void)context_p);
    // TODO the serial string is too big
}

static void
test_linq_receive_protocol_error_router(void** context_p)
{
    ((void)context_p);
    // TODO the serial string is too big
}

static void
test_linq_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_serial = "serial";
    linq* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("rid00", serial, "product", "site");

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_push_incoming(true);
    spy_sys_set_tick(100);

    // Receive a heartbeat
    linq_poll(l);
    device_s** d = linq_device(l, serial);
    assert_non_null(d);
    assert_int_equal(linq_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 4);
    assert_memory_equal(device_router(*d)->id, "rid0", 4);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_product(*d), "product");
    assert_int_equal(device_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    linq_poll(l);
    assert_non_null(d);
    assert_int_equal(linq_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 5);
    assert_memory_equal(device_router(*d)->id, "rid00", 5);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_product(*d), "product");
    assert_int_equal(device_uptime(*d), 100);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_heartbeat_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    linq* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* m = helpers_create_message_mem(
        4, "router", 6, "\x0", 1, "\x0", 1, "product", 7);

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_push_incoming(true);

    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_alert_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* sid = expect_serial = "sid";
    linq* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_push_incoming(true);

    linq_poll(l);
    pass = false;
    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_alert_error_short(void** context_p)
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
        cmocka_unit_test(test_linq_receive_protocol_error_short),
        cmocka_unit_test(test_linq_receive_protocol_error_serial),
        cmocka_unit_test(test_linq_receive_protocol_error_router),
        cmocka_unit_test(test_linq_receive_heartbeat_ok),
        cmocka_unit_test(test_linq_receive_heartbeat_error_short),
        cmocka_unit_test(test_linq_receive_alert_ok),
        cmocka_unit_test(test_linq_receive_alert_error_short),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
