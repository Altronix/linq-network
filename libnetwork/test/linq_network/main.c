// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "libcommon/device.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_utils.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "netw.h"
#include "zmtp_device.h"

#define USER "unsafe_user"
#define PASS "unsafe_pass"

static E_LINQ_ERROR expect_error = LINQ_ERROR_OK;
static const char* empty = "";
static const char* expect_what = "";
static const char* expect_sid = "";

#define TEST_RETRY_TIMEOUT 100
#define TEST_MAX_RETRY 5

static helpers_test_context_s*
test_init(helpers_test_config_s* config)
{
    zmtp_device_retry_timeout_set(TEST_RETRY_TIMEOUT);
    zmtp_device_max_retry_set(TEST_MAX_RETRY);
    return helpers_test_context_create(config);
}

static void
test_reset(helpers_test_context_s** test_p)
{
    helpers_test_context_destroy(test_p);
    expect_error = LINQ_ERROR_OK;
    expect_what = empty;
    expect_sid = empty;
    helpers_test_reset();
}

static void
test_error_fn(void* pass, E_LINQ_ERROR e, const char* serial, const char* what)
{
    assert_int_equal(e, expect_error);
    assert_string_equal(what, expect_what);
    assert_string_equal(serial, expect_sid);
    *((bool*)pass) = true;
}

static void
test_new_fn(void* pass, const char* serial)
{
    assert_string_equal(serial, expect_sid);
    *((bool*)pass) = true;
}

static void
test_heartbeat_fn(void* pass, const char* serial)
{
    assert_string_equal(serial, expect_sid);
    *((bool*)pass) = true;
}

static void
test_alert_fn(
    void* pass,
    const char* serial,
    netw_alert_s* alert,
    netw_email_s* email)
{
    assert_memory_equal(serial, expect_sid, strlen(expect_sid));
    assert_memory_equal(alert->who.p, "TestUser", 8);
    assert_memory_equal(alert->what.p, "TestAlert", 9);
    assert_memory_equal(alert->where.p, "Altronix Site ID", 16);
    assert_memory_equal(alert->when.p, "1", 1);
    assert_memory_equal(alert->mesg.p, "Test Alert Message", 18);
    assert_memory_equal(email->to0.p, "mail0@gmail.com", 15);
    assert_memory_equal(email->to1.p, "mail1@gmail.com", 15);
    assert_memory_equal(email->to2.p, "mail2@gmail.com", 15);
    assert_memory_equal(email->to3.p, "mail3@gmail.com", 15);
    assert_memory_equal(email->to4.p, "mail4@gmail.com", 15);
    *((bool*)pass) = true;
}

netw_callbacks callbacks = { .on_err = test_error_fn,
                             .on_new = test_new_fn,
                             .on_heartbeat = test_heartbeat_fn,
                             .on_alert = test_alert_fn };

static void
test_netw_create(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    helpers_test_context_s* test = helpers_test_context_create(&config);
    assert_non_null(test->net);
    helpers_test_context_destroy(&test);
}

static void
test_netw_receive_protocol_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    helpers_test_context_s* test = test_init(&config);
    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_protocol_error_callback_has_serial(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    // clang-format off
    zmsg_t* m = helpers_create_message_mem(
        5, 
        "router", 6, 
        "\x0", 1, 
        "\x8", 1, 
        "bar", 3, 
        "car", 3);
    // clang-format on

    helpers_test_context_s* test = test_init(&config);
    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));
    expect_sid = "bar";

    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_protocol_error_serial(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char sid[SID_LEN + 1];
    memset(sid, SID_LEN + 1, 'A');
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* m = helpers_create_message_mem(
        6, "rid", 3, "\x0", 1, "\x0", 1, sid, SID_LEN + 1, "pid", 3, "site", 4);

    helpers_test_context_s* test = test_init(&config);
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_protocol_error_router(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char rid[RID_LEN + 1];
    memset(rid, SID_LEN + 1, 'A');
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* m = helpers_create_message_mem(
        6, rid, RID_LEN + 1, "\x0", 1, "\x0", 1, "sid", 3, "pid", 3, "site", 4);

    helpers_test_context_s* test = test_init(&config);
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_sid = "serial";
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("rid00", serial, "product", "site");

    helpers_test_context_s* test = test_init(&config);

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_set_incoming((0x01));
    spy_sys_set_tick(100);

    // Receive a heartbeat
    netw_poll(test->net, 5);
    node_s** d = (node_s**)netw_device(test->net, serial);
    assert_non_null(d);
    assert_int_equal(netw_device_count(test->net), 1);
    assert_int_equal(zmtp_device_router(*d)->sz, 4);
    assert_memory_equal(zmtp_device_router(*d)->id, "rid0", 4);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    netw_poll(test->net, 5);
    assert_non_null(d);
    assert_int_equal(netw_device_count(test->net), 1);
    assert_int_equal(zmtp_device_router(*d)->sz, 5);
    assert_memory_equal(zmtp_device_router(*d)->id, "rid00", 5);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 100);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_heartbeat_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* m = helpers_create_message_mem(
        4, "router", 6, "\x0", 1, "\x0", 1, "sid", 3);

    helpers_test_context_s* test = test_init(&config);

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));
    expect_sid = "sid";

    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_alert_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* sid = expect_sid = "sid";
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");

    helpers_test_context_s* test = test_init(&config);

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    netw_poll(test->net, 5);
    pass = false;
    netw_poll(test->net, 5);

    assert_true(pass);

    test_reset(&test);
}

static void
test_netw_receive_alert_error_short(void** context_p)
{
    ((void)context_p);
}

static void
on_response_ok(void* pass, const char* serial, int err, const char* data)
{
    *(bool*)pass = true;
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(serial, "serial");
}

static void
test_netw_receive_response_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* serial = expect_sid = "serial";
    // zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, serial, "rid", "pid", "sid");

    czmq_spy_mesg_push_incoming(&r);
    czmq_spy_poll_set_incoming((0x01));

    // Receive heartbeat (add device to linq)
    // Send a get request
    // receive get response
    // make sure callback is as expect
    netw_send(
        test->net,
        serial,
        "GET",
        "/ATX/test",
        9,
        NULL,
        0,
        on_response_ok,
        &pass);
    netw_poll(test->net, 5);
    assert_true(pass);

    test_reset(&test);
}

static void
on_response_error_timeout(
    void* pass,
    const char* serial,
    int err,
    const char* data)
{
    *((bool*)pass) = true;
    assert_string_equal(serial, "serial");
    assert_int_equal(err, LINQ_ERROR_TIMEOUT);
    assert_string_equal(data, "{\"error\":\"timeout\"}");
}

static void
test_netw_receive_response_error_timeout(void** context_p)
{
    ((void)context_p);

    bool pass = false, response_pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* serial = expect_sid = "serial";
    node_s** d;
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    helpers_test_context_s* test = test_init(&config);

    // Receive a new device @t=0
    spy_sys_set_tick(0);
    helpers_add_device(test, serial, "rid0", "pid", "sid");
    czmq_spy_poll_set_incoming((0x01));
    d = netw_device(test->net, serial);
    zmtp_device_send(
        *d,
        REQUEST_METHOD_GET,
        "/ATX/test",
        9,
        NULL,
        0,
        on_response_error_timeout,
        &response_pass);
    assert_int_equal(zmtp_device_request_pending_count(*d), 1);

    // Still waiting for response @t=9999
    spy_sys_set_tick(TEST_RETRY_TIMEOUT - 1);
    czmq_spy_poll_set_incoming((0x00));
    netw_poll(test->net, 5);
    assert_false(response_pass);
    assert_int_equal(zmtp_device_request_pending_count(*d), 1);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        spy_sys_set_tick(TEST_RETRY_TIMEOUT * (1 + i));
        netw_poll(test->net, 5);
        assert_false(response_pass);
    }

    spy_sys_set_tick(TEST_RETRY_TIMEOUT * (1 + TEST_MAX_RETRY));
    netw_poll(test->net, 5);
    assert_true(response_pass);
    assert_int_equal(zmtp_device_request_pending_count(*d), 0);

    // Response is resolved but there is no more request pending
    czmq_spy_poll_set_incoming((0x01));
    czmq_spy_mesg_push_incoming(&r);
    netw_poll(test->net, 5);

    test_reset(&test);
}

static void
on_response_error_codes(
    void* pass,
    const char* serial,
    int err,
    const char* data)
{
    char expect[32];
    snprintf(expect, sizeof(expect), "{\"error\":%d}", expect_error);
    assert_string_equal(serial, "serial");
    assert_string_equal(data, expect);
    assert_int_equal(err, expect_error);
    *((bool*)pass) = true;
}

static void
test_netw_receive_response_error_codes(void** context_p)
{
    ((void)context_p);

    bool pass = false;
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* serial = expect_sid = "serial";
    node_s** d;

    int codes[] = { 0, 400, 403, 404, 500 };
    for (int i = 0; i < 5; i++) {
        // Setup incoming network (1st poll heartbeat, 2nd poll response)
        helpers_test_context_s* test = test_init(&config);
        // sqlite_spy_step_return_push(SQLITE_ROW);
        // sqlite_spy_column_int_return_push(1);

        char data[32];
        snprintf(data, sizeof(data), "{\"error\":%d}", codes[i]);
        expect_error = codes[i];
        zmsg_t* r = helpers_make_response("rid0", serial, codes[i], data);
        // Setup code under test
        helpers_add_device(test, serial, "rid0", "pid", "sid");
        d = netw_device(test->net, serial);
        assert_non_null(d);

        // Start test
        zmtp_device_send(
            *d,
            REQUEST_METHOD_GET,
            "/ATX/test",
            9,
            NULL,
            0,
            on_response_error_codes,
            &pass);
        assert_int_equal(zmtp_device_request_pending_count(*d), 1);

        czmq_spy_mesg_push_incoming(&r);
        czmq_spy_poll_set_incoming((0x01));
        netw_poll(test->net, 0);

        // Measure test
        assert_true(pass);

        // Cleanup test
        test_reset(&test);
    }
}

static void
on_response_error_504(void* pass, const char* serial, int err, const char* data)
{
    assert_string_equal(data, "{\"error\":504}");
    assert_int_equal(err, LINQ_ERROR_504);
    *((bool*)pass) = true;
}

static void
test_netw_receive_response_error_504(void** context_p)
{
    // Same as receive_response_ok_504 accept we add an extra 504 to incoming
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    uint32_t t = 0;
    const char* serial = expect_sid = "serial";
    node_s** d;
    zmsg_t* incoming[TEST_MAX_RETRY + 1] = {
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}")
    };
    zmsg_t* outgoing = NULL;

    helpers_test_context_s* test = test_init(&config);
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    // Setup code under test

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    helpers_add_device(test, serial, "rid0", "pid", "sid");
    d = netw_device(test->net, serial);
    assert_non_null(d);

    // Start test @t=0
    zmtp_device_send(
        *d,
        REQUEST_METHOD_GET,
        "/ATX/test",
        9,
        NULL,
        0,
        on_response_error_504,
        &pass);
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    zmsg_destroy(&outgoing);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        // incoming 504
        czmq_spy_mesg_push_incoming(&incoming[i]);
        czmq_spy_poll_set_incoming((0x01));
        netw_poll(test->net, 0);

        // @t=retry-1, make sure do not send request
        t += TEST_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        netw_poll(test->net, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_null(outgoing);

        // @t=retry, send request again, (incoming 504 #2)
        t++;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        netw_poll(test->net, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing); // TODO measure outgoing packets
        zmsg_destroy(&outgoing);
    }

    // Send the final amount of 504's we're willing to tollorate
    czmq_spy_mesg_push_incoming(&incoming[TEST_MAX_RETRY]);
    czmq_spy_poll_set_incoming(0x01);
    netw_poll(test->net, 0);

    assert_true(pass);
    test_reset(&test);
}

static void
on_response_ok_504(void* pass, const char* serial, int err, const char* data)
{
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(serial, "serial");
    *(bool*)pass = true;
}

static void
test_netw_receive_response_ok_504(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    uint32_t t = 0;
    const char* serial = expect_sid = "serial";
    node_s** d;
    zmsg_t* ok = helpers_make_response("rid0", serial, 0, "{\"test\":1}");
    zmsg_t* incoming[TEST_MAX_RETRY] = {
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}")
    };
    zmsg_t* outgoing = NULL;

    helpers_test_context_s* test = test_init(&config);
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    // Setup code under test

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    helpers_add_device(test, serial, "rid0", "pid", "sid");
    d = netw_device(test->net, serial);
    assert_non_null(d);

    // Start test @t=0
    zmtp_device_send(
        *d,
        REQUEST_METHOD_GET,
        "/ATX/test",
        9,
        NULL,
        0,
        on_response_ok_504,
        &pass);
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    zmsg_destroy(&outgoing);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        // incoming 504
        czmq_spy_mesg_push_incoming(&incoming[i]);
        czmq_spy_poll_set_incoming((0x01));
        netw_poll(test->net, 0);

        // @t=retry-1, make sure do not send request
        t += TEST_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        netw_poll(test->net, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_null(outgoing);

        // @t=retry, send request again, (incoming 504 #2)
        t++;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        netw_poll(test->net, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing); // TODO measure outgoing packets
        zmsg_destroy(&outgoing);
        assert_false(pass);
    }

    czmq_spy_mesg_push_incoming(&ok);
    czmq_spy_poll_set_incoming(0x01);
    netw_poll(test->net, 0);

    assert_true(pass);
    test_reset(&test);
}

static void
test_netw_receive_hello(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    zmsg_t* m = helpers_make_hello("router", "node");
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    assert_int_equal(netw_node_count(test->net), 0);
    netw_poll(test->net, 5);
    assert_int_equal(netw_node_count(test->net), 1);

    test_reset(&test);
}

static void
test_netw_receive_hello_double_id(void** context_p)
{
    ((void)context_p);
    zmsg_t* m0 = helpers_make_hello("router", "node");
    zmsg_t* m1 = helpers_make_hello("router", "node");
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_poll_set_incoming((0x01));

    assert_int_equal(netw_node_count(test->net), 0);
    netw_poll(test->net, 5);
    assert_int_equal(netw_node_count(test->net), 1);
    netw_poll(test->net, 5);
    assert_int_equal(netw_node_count(test->net), 1);

    test_reset(&test);
}

static void
test_netw_broadcast_heartbeat_receive(void** context_p)
{
    ((void)context_p);
    zmsg_t* hb = helpers_make_heartbeat(NULL, "serial", "product", "site");
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    netw_connect(test->net, "ipc:///123");

    // TODO - this heartbeat comes from a dealer socket
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));
    netw_poll(test->net, 5);

    test_reset(&test);
}

static void
test_netw_broadcast_heartbeat(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    zmsg_t* hb = helpers_make_heartbeat("rid0", "serial", "product", "site");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, "serial", "rid0", "pid", "sid");
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    // Client sends hello to server, device sends heartbeat to server
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    netw_poll(test->net, 5); // receive hello
    netw_poll(test->net, 5); // recieve hello
    netw_poll(test->net, 5); // recieve heartbeat

    // outgoing should have a heartbeat with client router
    for (int i = 0; i < 2; i++) {
        zframe_t *rid, *ver, *typ, *sid, *pid, *loc;
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing);
        rid = zmsg_pop(outgoing);
        ver = zmsg_pop(outgoing);
        typ = zmsg_pop(outgoing);
        sid = zmsg_pop(outgoing);
        pid = zmsg_pop(outgoing);
        loc = zmsg_pop(outgoing);
        assert_memory_equal(zframe_data(rid), "client-router", 13);
        assert_memory_equal(zframe_data(ver), "\x0", 1);
        assert_memory_equal(zframe_data(typ), "\x80", 1);
        assert_memory_equal(zframe_data(sid), "serial", 6);
        assert_memory_equal(zframe_data(pid), "product", 6);
        assert_memory_equal(zframe_data(loc), "site", 4);
        zmsg_destroy(&outgoing);
        zframe_destroy(&rid);
        zframe_destroy(&ver);
        zframe_destroy(&typ);
        zframe_destroy(&sid);
        zframe_destroy(&pid);
        zframe_destroy(&loc);
    }

    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_null(outgoing);

    test_reset(&test);
}

static void
test_netw_broadcast_alert(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    zmsg_t* alert = helpers_make_alert("rid", "sid", "pid");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, "sid", "rid0", "pid", "sid");
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    // device sends heartbeat to server, two clients connect, device sends alert
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    netw_poll(test->net, 5); // receive hello
    netw_poll(test->net, 5); // recieve hello
    netw_poll(test->net, 5); // receive alert

    // outgoing should have a heartbeat with client router
    for (int i = 0; i < 2; i++) {
        zframe_t *rid, *ver, *typ, *sid, *pid, *alert, *email;
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing);
        rid = zmsg_pop(outgoing);
        ver = zmsg_pop(outgoing);
        typ = zmsg_pop(outgoing);
        sid = zmsg_pop(outgoing);
        pid = zmsg_pop(outgoing);
        alert = zmsg_pop(outgoing);
        email = zmsg_pop(outgoing);
        assert_memory_equal(zframe_data(rid), "client-router", 13);
        assert_memory_equal(zframe_data(ver), "\x0", 1);
        assert_memory_equal(zframe_data(typ), "\x83", 1);
        assert_memory_equal(zframe_data(sid), "sid", 3);
        assert_memory_equal(zframe_data(pid), "pid", 3);
        assert_memory_equal(zframe_data(alert), TEST_ALERT, strlen(TEST_ALERT));
        assert_memory_equal(zframe_data(email), TEST_EMAIL, strlen(TEST_EMAIL));
        zmsg_destroy(&outgoing);
        zframe_destroy(&rid);
        zframe_destroy(&ver);
        zframe_destroy(&typ);
        zframe_destroy(&sid);
        zframe_destroy(&pid);
        zframe_destroy(&alert);
        zframe_destroy(&email);
    }

    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_null(outgoing);

    test_reset(&test);
}

static void
test_netw_forward_request(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t *hb, *hello, *request, *response, *outgoing;
    zframe_t *rid, *ver, *typ, *sid, *url, *err, *dat;
    hb = helpers_make_heartbeat("router-d", "device123", "pid", "site");
    hello = helpers_make_hello("router-c", "client123");
    request = helpers_make_request("router-c", "device123", "GET /hello", NULL);
    response = helpers_make_response("router-d", "device123", 0, "world");

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, "device123", "router-d", "pid", "site");
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);       // device heartbeat
    czmq_spy_mesg_push_incoming(&hello);    // remote client hello
    czmq_spy_mesg_push_incoming(&request);  // remote client request
    czmq_spy_mesg_push_incoming(&response); // device response
    czmq_spy_poll_set_incoming((0x01));

    netw_poll(test->net, 5);
    netw_poll(test->net, 5);
    netw_poll(test->net, 5);
    netw_poll(test->net, 5);

    // First outgoing message is to the device
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    assert_int_equal(zmsg_size(outgoing), 5);
    rid = zmsg_pop(outgoing);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    sid = zmsg_pop(outgoing);
    url = zmsg_pop(outgoing);

    assert_memory_equal(zframe_data(rid), "router-d", 8);
    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x1", 1);
    assert_memory_equal(zframe_data(sid), "device123", 9);
    assert_memory_equal(zframe_data(url), "GET /hello", 10);
    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zframe_destroy(&url);
    zmsg_destroy(&outgoing);

    // Second outgoing message is the response back to the remote client
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    assert_int_equal(zmsg_size(outgoing), 6);
    rid = zmsg_pop(outgoing);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    sid = zmsg_pop(outgoing);
    err = zmsg_pop(outgoing);
    dat = zmsg_pop(outgoing);
    assert_memory_equal(zframe_data(rid), "router-c", 8);
    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x2", 1);
    assert_memory_equal(zframe_data(sid), "device123", 9);
    assert_memory_equal(zframe_data(err), "\x0", 1);
    assert_memory_equal(zframe_data(dat), "world", 5);
    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zframe_destroy(&err);
    zframe_destroy(&dat);
    zmsg_destroy(&outgoing);

    test_reset(&test);
}

static void
test_netw_forward_client_request(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zframe_t *ver, *typ, *sid, *url;
    zmsg_t* hb = helpers_make_heartbeat(NULL, "device123", "pid", "site");
    zmsg_t* a = helpers_make_response(NULL, "device123", 0, "{\"about\":null}");
    zmsg_t* outgoing = NULL;

    helpers_test_context_s* test = test_init(&config);
    // sqlite_spy_step_return_push(SQLITE_ROW);
    // sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&a);
    czmq_spy_poll_set_incoming(0x01);

    netw_connect(test->net, "ipc:///test");

    netw_poll(test->net, 5); // add a device
    netw_poll(test->net, 5);
    czmq_spy_mesg_flush_outgoing(); // delete hello frames and /about request

    netw_send(
        test->net, "device123", "GET", "/ATX/hello", 10, NULL, 0, NULL, NULL);
    // outgoing = czmq_spy_mesg_pop_outgoing();
    // assert_non_null(outgoing);
    // zmsg_destroy(&outgoing); // delete outgoing hello frames
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    assert_int_equal(zmsg_size(outgoing), 4);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    sid = zmsg_pop(outgoing);
    url = zmsg_pop(outgoing);
    assert_memory_equal(zframe_data(ver), &g_frame_ver_0, 1);
    assert_memory_equal(zframe_data(typ), &g_frame_typ_request, 1);
    assert_memory_equal(zframe_data(sid), "device123", 9);
    assert_memory_equal(zframe_data(url), "GET /ATX/hello", 14);

    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zframe_destroy(&url);
    zmsg_destroy(&outgoing);
    test_reset(&test);
}

static void
test_netw_connect(void** context_p)
{
    ((void)context_p);
    netw_socket s;

    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    helpers_test_context_s* test = test_init(&config);

    s = netw_connect(test->net, "ipc:///filex");
    assert_true(!(LINQ_ERROR_OK == s));

    zmsg_t* outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    assert_int_equal(zmsg_size(outgoing), 3);
    zframe_t* ver = zmsg_pop(outgoing);
    zframe_t* typ = zmsg_pop(outgoing);
    zframe_t* sid = zmsg_pop(outgoing);

    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x4", 1);
    assert_int_equal(zframe_size(sid), 12);
    assert_memory_equal(zframe_data(sid), "ipc:///filex", 12);

    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&sid);
    zmsg_destroy(&outgoing);

    test_reset(&test);
}

static void
test_netw_close_router(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    netw_socket l0 = netw_listen(test->net, "tcp://1.2.3.4:8080");
    netw_socket l1 = netw_listen(test->net, "tcp://5.6.7.8:8080");
    netw_socket c0 = netw_connect(test->net, "tcp://11.22.33.44:8888");
    netw_socket c1 = netw_connect(test->net, "tcp://55.66.77.88:8888");
    zmsg_t* hb0 = helpers_make_heartbeat("r0", "dev1", "pid", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("r1", "dev2", "pid", "site");
    zmsg_t* hb2 = helpers_make_heartbeat(NULL, "dev3", "pid", "site");
    zmsg_t* hb3 = helpers_make_heartbeat(NULL, "dev4", "pid", "site");
    zmsg_t* hb4 = helpers_make_heartbeat("r5", "dev5", "pid", "site");
    zmsg_t* hb5 = helpers_make_heartbeat("r6", "dev6", "pid", "site");
    zmsg_t* hb6 = helpers_make_heartbeat(NULL, "dev7", "pid", "site");
    zmsg_t* hb7 = helpers_make_heartbeat(NULL, "dev8", "pid", "site");

    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_mesg_push_incoming(&hb2);
    czmq_spy_mesg_push_incoming(&hb3);
    czmq_spy_mesg_push_incoming(&hb4);
    czmq_spy_mesg_push_incoming(&hb5);
    czmq_spy_mesg_push_incoming(&hb6);
    czmq_spy_mesg_push_incoming(&hb7);
    czmq_spy_poll_set_incoming((0b1111)); // Knowledge of innards
    netw_poll(test->net, 5);
    netw_poll(test->net, 5);
    assert_int_equal(netw_device_count(test->net), 8);
    netw_close(test->net, l0);
    assert_int_equal(netw_device_count(test->net), 6);
    netw_close(test->net, l1);
    assert_int_equal(netw_device_count(test->net), 4);
    netw_close(test->net, c0);
    assert_int_equal(netw_device_count(test->net), 2);
    assert_int_equal(netw_node_count(test->net), 1);
    netw_close(test->net, c1);
    assert_int_equal(netw_device_count(test->net), 0);
    assert_int_equal(netw_node_count(test->net), 0);

    test_reset(&test);
}

static void
test_netw_devices_callback(void* context, const char* sid, const char* pid)
{
    uint32_t *mask = context, idx = 0;
    assert_memory_equal(sid, "dev", 3);
    assert_memory_equal(pid, "pid", 3);
    idx = atoi(&sid[3]);
    *mask |= (0x01 << idx);
    assert_int_equal(idx, atoi(&pid[3]));
}

/*
static void
test_netw_devices_foreach(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    netw_listen(test->net, "tcp://1.2.3.4:8080");
    netw_listen(test->net, "tcp://5.6.7.8:8080");
    netw_connect(test->net, "tcp://11.22.33.44:8888");
    netw_connect(test->net, "tcp://55.66.77.88:8888");
    zmsg_t* hb0 = helpers_make_heartbeat("r0", "dev0", "pid0", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("r1", "dev1", "pid1", "site");
    zmsg_t* hb2 = helpers_make_heartbeat(NULL, "dev2", "pid2", "site");
    zmsg_t* hb3 = helpers_make_heartbeat(NULL, "dev3", "pid3", "site");
    zmsg_t* hb4 = helpers_make_heartbeat("r5", "dev4", "pid4", "site");
    zmsg_t* hb5 = helpers_make_heartbeat("r6", "dev5", "pid5", "site");
    zmsg_t* hb6 = helpers_make_heartbeat(NULL, "dev6", "pid6", "site");
    zmsg_t* hb7 = helpers_make_heartbeat(NULL, "dev7", "pid7", "site");

    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_mesg_push_incoming(&hb2);
    czmq_spy_mesg_push_incoming(&hb3);
    czmq_spy_mesg_push_incoming(&hb4);
    czmq_spy_mesg_push_incoming(&hb5);
    czmq_spy_mesg_push_incoming(&hb6);
    czmq_spy_mesg_push_incoming(&hb7);
    czmq_spy_poll_set_incoming((0b1111)); // Knowledge of innards
    netw_poll(test->net, 5);
    netw_poll(test->net, 5);
    assert_int_equal(netw_device_count(test->net), 8);

    uint32_t mask = 0x00;
    netw_devices_foreach(test->net, test_netw_devices_callback, &mask);
    assert_int_equal(mask, 0b11111111);

    test_reset(&test);
}
*/

static void
test_netw_device_remove(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    int ret;
    const char* serial = expect_sid = "serial";
    helpers_test_config_s config = { .callbacks = &callbacks,
                                     .context = &pass,
                                     .zmtp = 32820,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");

    helpers_test_context_s* test = test_init(&config);

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_poll_set_incoming((0x01));
    spy_sys_set_tick(100);
    assert_int_equal(netw_device_count(test->net), 0);

    // Receive a heartbeat
    netw_poll(test->net, 5);
    node_s** d = (node_s**)netw_device(test->net, serial);
    assert_non_null(d);
    assert_int_equal(netw_device_count(test->net), 1);

    ret = netw_device_remove(test->net, serial);
    assert_int_equal(ret, 0);
    assert_int_equal(netw_device_count(test->net), 0);
    ret = netw_device_remove(test->net, serial);
    assert_int_equal(ret, -1);

    assert_true(pass);

    test_reset(&test);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_netw_create),
        cmocka_unit_test(test_netw_receive_protocol_error_short),
        cmocka_unit_test(test_netw_receive_protocol_error_callback_has_serial),
        cmocka_unit_test(test_netw_receive_protocol_error_serial),
        cmocka_unit_test(test_netw_receive_protocol_error_router),
        cmocka_unit_test(test_netw_receive_heartbeat_ok),
        cmocka_unit_test(test_netw_receive_heartbeat_error_short),
        cmocka_unit_test(test_netw_receive_alert_ok),
        cmocka_unit_test(test_netw_receive_alert_error_short),
        cmocka_unit_test(test_netw_receive_response_ok),
        cmocka_unit_test(test_netw_receive_response_error_timeout),
        cmocka_unit_test(test_netw_receive_response_error_codes),
        cmocka_unit_test(test_netw_receive_response_error_504),
        cmocka_unit_test(test_netw_receive_response_ok_504),
        cmocka_unit_test(test_netw_receive_hello),
        cmocka_unit_test(test_netw_receive_hello_double_id),
        cmocka_unit_test(test_netw_broadcast_heartbeat),
        cmocka_unit_test(test_netw_broadcast_heartbeat_receive),
        cmocka_unit_test(test_netw_broadcast_alert),
        cmocka_unit_test(test_netw_forward_request),
        cmocka_unit_test(test_netw_forward_client_request),
        cmocka_unit_test(test_netw_connect),
        cmocka_unit_test(test_netw_close_router),
        cmocka_unit_test(test_netw_device_remove),
        // cmocka_unit_test(test_netw_devices_foreach)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
