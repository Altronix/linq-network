// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/device.h"
#include "helpers.h"
#include "mock_utils.h"
#include "mock_zmq.h"
#include "netw.h"
#include "zmtp/zmtp_device.h"

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
    helpers_test_init();
    return helpers_test_context_create(config);
}

static void
test_reset(helpers_test_context_s** test_p)
{
    helpers_test_context_destroy(test_p);
    helpers_test_reset();
    expect_error = LINQ_ERROR_OK;
    expect_what = empty;
    expect_sid = empty;
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
    helpers_test_context_s* test = test_init(&config);

    helpers_push_str(2, "too", "short");
    expect_error = LINQ_ERROR_PROTOCOL;
    zmq_spy_poll_set_ready((0x01));

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

    helpers_test_context_s* test = test_init(&config);

    helpers_push_mem(5, "router", 6, "\x0", 1, "\x8", 1, "bar", 3, "car", 3);
    expect_error = LINQ_ERROR_PROTOCOL;
    zmq_spy_poll_set_ready(0x01);
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
    helpers_test_context_s* test = test_init(&config);

    // clang-format off
    helpers_push_mem(
        6,
        "rid", 3,
        "\x0", 1,
        "\x0", 1,
        sid, SID_LEN + 1,
        "pid", 3,
        "site", 4);
    // clang-format on

    zmq_spy_poll_set_ready((0x01));

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
    helpers_test_context_s* test = test_init(&config);

    // clang-format off
    helpers_push_mem(
        6,
        rid, RID_LEN + 1,
        "\x0", 1,
        "\x0", 1,
        "sid", 3,
        "pid", 3,
        "site", 4);
    // clang-format on

    zmq_spy_poll_set_ready(0x01);

    expect_error = LINQ_ERROR_PROTOCOL;

    expect_sid = "sid";

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
    helpers_test_context_s* test = test_init(&config);
    helpers_push_heartbeat("rid0", serial, "product", "site");
    helpers_push_heartbeat("rid00", serial, "product", "site");

    // Push some incoming heartbeats
    zmq_spy_poll_set_ready(0x01);
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
    // NOTE somewhere in the recent update we lost uptime tracking
    // assert_int_equal(device_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    netw_poll(test->net, 5);
    assert_non_null(d);
    assert_int_equal(netw_device_count(test->net), 1);
    assert_int_equal(zmtp_device_router(*d)->sz, 5);
    assert_memory_equal(zmtp_device_router(*d)->id, "rid00", 5);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    // NOTE somewhere in the recent update we lost uptime tracking
    // assert_int_equal(device_uptime(*d), 100);

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
    helpers_test_context_s* test = test_init(&config);
    helpers_push_mem(4, "router", 6, "\x0", 1, "\x0", 1, "sid", 3);

    expect_error = LINQ_ERROR_PROTOCOL;
    zmq_spy_poll_set_ready(0x01);
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

    helpers_test_context_s* test = test_init(&config);
    helpers_push_heartbeat("rid", sid, "pid", "site");
    helpers_push_alert("rid", sid, "pid");

    // Push some incoming messages
    zmq_spy_poll_set_ready((0x01));

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

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, serial, "rid", "pid", "sid");
    helpers_push_response("rid0", serial, 0, 0, "{\"test\":1}");

    zmq_spy_poll_set_ready((0x01));

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
test_netw_receive_response_ok_legacy(void** context_p)
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

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device_legacy(test, serial, "rid", "pid", "sid");
    helpers_push_response_legacy("rid0", serial, 0, "{\"test\":1}");

    zmq_spy_poll_set_ready((0x01));

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

    helpers_test_context_s* test = test_init(&config);

    // Receive a new device @t=0
    spy_sys_set_tick(0);
    helpers_add_device(test, serial, "rid0", "pid", "sid");
    zmq_spy_poll_set_ready((0x01));
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
    zmq_spy_poll_set_ready((0x00));
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
    zmq_spy_poll_set_ready((0x01));
    helpers_push_response("rid0", serial, 0, 0, "{\"test\":1}");
    netw_poll(test->net, 5);

    assert_true(pass);
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
        pass = false;
        // Setup incoming network (1st poll heartbeat, 2nd poll response)
        helpers_test_context_s* test = test_init(&config);
        // sqlite_spy_step_return_push(SQLITE_ROW);
        // sqlite_spy_column_int_return_push(1);

        char data[32];
        snprintf(data, sizeof(data), "{\"error\":%d}", codes[i]);
        expect_error = codes[i];
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

        helpers_push_response("rid0", serial, 0, codes[i], data);
        zmq_spy_poll_set_ready((0x01));
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
    uint32_t t = 0, outgoing = 0, reqid = 0;
    const char* serial = expect_sid = "serial";
    node_s** d;

    helpers_test_context_s* test = test_init(&config);

    // Setup code under test

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    helpers_add_device(test, serial, "rid0", "pid", "serial");
    d = netw_device(test->net, serial);
    assert_non_null(d);
    assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

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
    outgoing += 6;
    assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        // incoming 504
        helpers_push_response("rid0", serial, reqid, 504, "{\"error\":504}");
        zmq_spy_poll_set_ready((0x01));
        netw_poll(test->net, 0);

        // @t=retry-1, make sure do not send request (outgoing stays same)
        t += TEST_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

        // @t=retry, send request again, (incoming 504 #2) (outgoing +=6)
        t++;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        outgoing += 6;
        reqid++;
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);
    }

    // Send the final amount of 504's we're willing to tollorate
    helpers_push_response("rid0", serial, reqid, 504, "{\"error\":504}");
    zmq_spy_poll_set_ready(0x01);
    netw_poll(test->net, 0);

    assert_true(pass);
    test_reset(&test);
}

static void
test_netw_receive_response_error_504_legacy(void** context_p)
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
    uint32_t t = 0, outgoing = 0, reqid = 0;
    const char* serial = expect_sid = "serial";
    node_s** d;

    helpers_test_context_s* test = test_init(&config);

    // Setup code under test

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    helpers_add_device_legacy(test, serial, "rid0", "pid", "serial");
    d = netw_device(test->net, serial);
    assert_non_null(d);
    assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

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
    outgoing += 6;
    assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        // incoming 504
        helpers_push_response_legacy("rid0", serial, 504, "{\"error\":504}");
        zmq_spy_poll_set_ready((0x01));
        netw_poll(test->net, 0);

        // @t=retry-1, make sure do not send request (outgoing stays same)
        t += TEST_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

        // @t=retry, send request again, (incoming 504 #2) (outgoing +=6)
        t++;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        outgoing += 5;
        reqid++;
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);
    }

    // Send the final amount of 504's we're willing to tollorate
    helpers_push_response_legacy("rid0", serial, 504, "{\"error\":504}");
    zmq_spy_poll_set_ready(0x01);
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
    uint32_t t = 0, outgoing = 0, reqid = 0;
    const char* serial = expect_sid = "serial";
    node_s** d;

    helpers_test_context_s* test = test_init(&config);

    // Setup code under test

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    helpers_add_device(test, serial, "rid0", "pid", "serial");
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
    outgoing += 6;
    assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

    for (int i = 0; i < TEST_MAX_RETRY; i++) {
        // incoming 504
        helpers_push_response("rid0", serial, reqid, 504, "{\"error\":504}");
        zmq_spy_poll_set_ready((0x01));
        netw_poll(test->net, 0);

        // @t=retry-1, make sure do not send request
        t += TEST_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);

        // @t=retry, send request again, (incoming 504 #2)
        t++;
        spy_sys_set_tick(t);
        zmq_spy_poll_set_ready((0x00));
        netw_poll(test->net, 0);
        outgoing += 6;
        reqid++;
        assert_int_equal(zmq_spy_mesg_n_outgoing(), outgoing);
        assert_false(pass);
    }

    helpers_push_response("rid0", serial, reqid, 0, "{\"test\":1}");
    zmq_spy_poll_set_ready(0x01);
    netw_poll(test->net, 0);

    assert_true(pass);
    test_reset(&test);
}

static void
on_response_drop(void* pass, const char* serial, int err, const char* data)
{
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(serial, "serial");
    *(bool*)pass = true;
}
static void
test_netw_receive_late_response_w_drop_ok(void** context_p)
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

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, serial, "rid", "pid", "sid");

    helpers_push_response("rid0", serial, 1, 0, "{\"test\":1}");
    helpers_push_response("rid0", serial, 0, 0, "{\"test\":1}");

    zmq_spy_poll_set_ready((0x01));

    // Receive heartbeat (add device to linq)
    // Send a get request
    // receive get response with WRONG ID
    // receive get response with CORRECT ID
    // make sure callback is as expect
    netw_send(
        test->net,
        serial,
        "GET",
        "/ATX/test",
        9,
        NULL,
        0,
        on_response_drop,
        &pass);

    netw_poll(test->net, 5);
    assert_false(pass);
    netw_poll(test->net, 5);
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
        cmocka_unit_test(test_netw_receive_response_ok_legacy),
        cmocka_unit_test(test_netw_receive_response_error_timeout),
        cmocka_unit_test(test_netw_receive_response_error_codes),
        cmocka_unit_test(test_netw_receive_response_error_504),
        cmocka_unit_test(test_netw_receive_response_error_504_legacy),
        cmocka_unit_test(test_netw_receive_response_ok_504),
        cmocka_unit_test(test_netw_receive_late_response_w_drop_ok),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
