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
test_device_create(void** context_p)
{
    ((void)context_p);
    zsock_t* sock = NULL;
    device_s* d = device_create(&sock, (uint8_t*)"rid", 3, "sid", "pid");
    assert_non_null(d);

    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_post(d, "ATX", "{\"test\":1}", NULL);
    assert_int_equal(device_request_pending_count(d), 5);

    device_destroy(&d);
    assert_null(d);
    test_reset();
}

static void
test_device_send_get_no_prefix(void** context_p)
{
    ((void)context_p);
    zsock_t* sock = NULL;
    device_s* d = device_create(&sock, (uint8_t*)"rid", 3, "sid", "pid");
    zmsg_t* msg;
    zframe_t *rid, *ver, *typ, *sid, *url;

    device_send_get(d, "ATX/hardware", NULL);
    msg = czmq_spy_mesg_pop_outgoing();
    rid = zmsg_pop(msg);
    ver = zmsg_pop(msg);
    typ = zmsg_pop(msg);
    sid = zmsg_pop(msg);
    url = zmsg_pop(msg);
    zmsg_destroy(&msg);

    assert_string_equal(zframe_data(rid), "rid");
    assert_string_equal(zframe_data(ver), "");
    assert_string_equal(zframe_data(typ), "\x1");
    assert_string_equal(zframe_data(sid), "sid");
    assert_string_equal(zframe_data(url), "GET /ATX/hardware");
    assert_int_equal(device_request_pending_count(d), 1);

    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zframe_destroy(&url);

    device_destroy(&d);
    test_reset();
}

static void
test_device_send_get_with_prefix(void** context_p)
{
    ((void)context_p);
    zsock_t* sock = NULL;
    device_s* d = device_create(&sock, (uint8_t*)"rid", 3, "sid", "pid");
    zmsg_t* msg;
    zframe_t *rid, *ver, *typ, *sid, *url;

    device_send_get(d, "/ATX/hardware", NULL);
    msg = czmq_spy_mesg_pop_outgoing();
    rid = zmsg_pop(msg);
    ver = zmsg_pop(msg);
    typ = zmsg_pop(msg);
    sid = zmsg_pop(msg);
    url = zmsg_pop(msg);
    zmsg_destroy(&msg);

    assert_string_equal(zframe_data(rid), "rid");
    assert_string_equal(zframe_data(ver), "");
    assert_string_equal(zframe_data(typ), "\x1");
    assert_string_equal(zframe_data(sid), "sid");
    assert_string_equal(zframe_data(url), "GET /ATX/hardware");
    assert_int_equal(device_request_pending_count(d), 1);

    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zframe_destroy(&url);

    device_destroy(&d);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_device_create),
        cmocka_unit_test(test_device_send_get_no_prefix),
        cmocka_unit_test(test_device_send_get_with_prefix),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
