// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "mock_utils.h"
#include "mock_zmq.h"
#include "netw.h"
#include "zmtp/zmtp_device.h"

#define device_send_get(d, path, cb, ctx)                                      \
    zmtp_device_send(                                                          \
        d, REQUEST_METHOD_GET, path, strlen(path), NULL, 0, cb, ctx)
#define device_send_post(d, path, data, cb, ctx)                               \
    zmtp_device_send(                                                          \
        d,                                                                     \
        REQUEST_METHOD_POST,                                                   \
        path,                                                                  \
        strlen(path),                                                          \
        data,                                                                  \
        strlen(data),                                                          \
        cb,                                                                    \
        ctx)
#define device_send_delete(d, path, cb, ctx)                                   \
    zmtp_device_send(                                                          \
        d, REQUEST_METHOD_DELETE, path, strlen(path), NULL, 0, cb, ctx)

#define check_message(idx, more, mem, len)                                     \
    assert_msg_equal(&zmq_spy_mesg_at_outgoing(idx)->msg, more, mem, len)

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
    zmq_spy_flush();
}

static void
test_stub(void** context_p)
{
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = "unsafe_user",
                                     .pass = "unsafe_pass" };
    helpers_test_context_s* ctx = helpers_test_context_create(&config);
    helpers_test_context_destroy(&ctx);
}

static void
test_device_create(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");
    assert_non_null(d);

    device_send_get(d, "ATX", NULL, NULL);
    device_send_get(d, "ATX", NULL, NULL);
    device_send_get(d, "ATX", NULL, NULL);
    device_send_get(d, "ATX", NULL, NULL);
    device_send_post(d, "ATX", "{\"test\":1}", NULL, NULL);
    assert_int_equal(zmtp_device_request_pending_count(d), 5);

    zmtp_device_destroy(&d);
    assert_null(d);
    test_reset();
}

static void
test_device_send_get_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_get(d, "ATX/hardware", NULL, NULL);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);
    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, DONE, "GET /ATX/hardware", 17);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_get_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_get(d, "/ATX/hardware", NULL, NULL);

    assert_int_equal(zmtp_device_request_pending_count(d), 1);
    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, DONE, "GET /ATX/hardware", 17);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_delete_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_delete(d, "ATX/hardware", NULL, NULL);

    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, DONE, "DELETE /ATX/hardware", 20);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_delete_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_delete(d, "/ATX/hardware", NULL, NULL);

    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, DONE, "DELETE /ATX/hardware", 20);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_post_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_post(d, "ATX/hardware", "{\"test\":1}", NULL, NULL);

    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, MORE, "POST /ATX/hardware", 18);
    check_message(6, DONE, "{\"test\":1}", 10);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_post_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");

    device_send_post(d, "/ATX", "{\"test\":1}", NULL, NULL);
    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, MORE, "POST /ATX", 9);
    check_message(6, DONE, "{\"test\":1}", 10);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_get_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");
    device_send_get(d, "ATX/hardware", NULL, NULL);

    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, DONE, "GET /ATX/hardware", 17);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_get_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");

    device_send_get(d, "/ATX/hardware", NULL, NULL);

    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, DONE, "GET /ATX/hardware", 17);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_delete_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");

    device_send_delete(d, "ATX/hardware", NULL, NULL);

    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, DONE, "DELETE /ATX/hardware", 20);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_delete_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");

    device_send_delete(d, "/ATX/hardware", NULL, NULL);

    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, DONE, "DELETE /ATX/hardware", 20);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_post_no_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");

    device_send_post(d, "ATX/hardware", "{\"test\":1}", NULL, NULL);

    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, MORE, "POST /ATX/hardware", 18);
    check_message(5, DONE, "{\"test\":1}", 10);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_send_hop_post_with_prefix(void** context_p)
{
    ((void)context_p);
    node_s* d = zmtp_device_create(NULL, NULL, 3, "sid", "pid");

    device_send_post(d, "/ATX", "{\"test\":1}", NULL, NULL);
    check_message(0, MORE, "\x0", 1);
    check_message(1, MORE, "\x1", 1);
    check_message(2, MORE, "sid", 3);
    check_message(3, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(4, MORE, "POST /ATX", 9);
    check_message(5, DONE, "{\"test\":1}", 10);
    assert_int_equal(zmtp_device_request_pending_count(d), 1);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
test_device_request_id_increment(void** context_p)
{
    ((void)context_p);

    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");
    device_send_get(d, "ATX/hardware", NULL, NULL);
    zmtp_device_request_resolve(d, 0, "{\"test\":1}");
    device_send_get(d, "ATX/hardware", NULL, NULL);
    zmtp_device_request_resolve(d, 0, "{\"test\":1}");

    check_message(0, MORE, "rid", 3);
    check_message(1, MORE, "\x0", 1);
    check_message(2, MORE, "\x1", 1);
    check_message(3, MORE, "sid", 3);
    check_message(4, MORE, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    check_message(5, DONE, "GET /ATX/hardware", 17);

    check_message(6, MORE, "rid", 3);
    check_message(7, MORE, "\x0", 1);
    check_message(8, MORE, "\x1", 1);
    check_message(9, MORE, "sid", 3);
    check_message(10, MORE, "\x00\x00\x00\x00\x00\x00\x00\x01", 8);
    check_message(11, DONE, "GET /ATX/hardware", 17);

    zmtp_device_destroy(&d);
    test_reset();
}

static void
on_response(void* context, const char* serial, E_LINQ_ERROR e, const char* json)
{
    bool* pass = context;
    *pass = true;
    assert_string_equal(json, "{\"test\":1}");
    assert_string_equal("sid", serial);
    assert_int_equal(e, 0);
}

static void
test_device_response(void** context_p)
{
    ((void)context_p);
    bool* pass = false;
    node_s* d = zmtp_device_create(NULL, (uint8_t*)"rid", 3, "sid", "pid");
    device_send_post(d, "/ATX/hardware", "{\"test\":1}", on_response, &pass);

    // TODO - add similiar test from linq.c which will test the parsing
    assert_int_equal(zmtp_device_request_pending_count(d), 1);
    zmtp_device_request_resolve(d, 0, "{\"test\":1}");
    assert_int_equal(zmtp_device_request_pending_count(d), 0);
    assert_true(pass);

    zmtp_device_destroy(&d);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_stub),
        cmocka_unit_test(test_device_create),
        cmocka_unit_test(test_device_send_get_no_prefix),
        cmocka_unit_test(test_device_send_get_with_prefix),
        cmocka_unit_test(test_device_send_delete_no_prefix),
        cmocka_unit_test(test_device_send_delete_with_prefix),
        cmocka_unit_test(test_device_send_post_no_prefix),
        cmocka_unit_test(test_device_send_post_with_prefix),
        cmocka_unit_test(test_device_send_hop_get_no_prefix),
        cmocka_unit_test(test_device_send_hop_get_with_prefix),
        cmocka_unit_test(test_device_send_hop_delete_no_prefix),
        cmocka_unit_test(test_device_send_hop_delete_with_prefix),
        cmocka_unit_test(test_device_send_hop_post_no_prefix),
        cmocka_unit_test(test_device_send_hop_post_with_prefix),
        cmocka_unit_test(test_device_request_id_increment),
        cmocka_unit_test(test_device_response),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

