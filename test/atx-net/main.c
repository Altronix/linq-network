// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/atx_net.h"
#include "atx_net_internal.h"
#include "device.h"
#include "helpers.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#ifdef WITH_SQLITE
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#endif

#include <setjmp.h>

#include <cmocka.h>

static E_LINQ_ERROR expect_error = LINQ_ERROR_OK;
static const char* empty = "";
static const char* expect_what = "";
static const char* expect_sid = "";

static void
test_init()
{
#ifdef WITH_SQLITE
    mongoose_spy_init();
    sqlite_spy_init();
    sqlite_spy_step_return_push(SQLITE_DONE); // PRAGMA

    // TODO use header to define how many tables there are
    sqlite_spy_step_return_push(SQLITE_ROW);  // device database OK
    sqlite_spy_step_return_push(SQLITE_ROW);  // alert database OK
    sqlite_spy_step_return_push(SQLITE_ROW);  // users database OK
#endif
}

static void
test_reset()
{
    expect_error = LINQ_ERROR_OK;
    expect_what = empty;
    expect_sid = empty;
    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
#ifdef WITH_SQLITE
    mongoose_spy_deinit();
    sqlite_spy_deinit();
#endif
}

static void
atx_net_on_error_fn(
    void* pass,
    E_LINQ_ERROR e,
    const char* what,
    const char* serial)
{
    assert_int_equal(e, expect_error);
    assert_string_equal(what, expect_what);
    assert_string_equal(serial, expect_sid);
    *((bool*)pass) = true;
}

static void
atx_net_on_heartbeat_fn(void* pass, const char* serial, device_s** d)
{
    assert_string_equal(serial, expect_sid);
    assert_string_equal(device_serial(*d), expect_sid);
    *((bool*)pass) = true;
}

static void
atx_net_on_alert_fn(
    void* pass,
    atx_net_alert_s* alert,
    atx_net_email_s* email,
    device_s** d)
{
    assert_memory_equal(device_serial(*d), expect_sid, strlen(expect_sid));
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

atx_net_callbacks callbacks = { .err = atx_net_on_error_fn,
                                .hb = atx_net_on_heartbeat_fn,
                                .alert = atx_net_on_alert_fn };

static void
test_atx_net_create(void** context_p)
{
    ((void)context_p);
    test_init();
    atx_net_s* l = atx_net_create(NULL, NULL);
    assert_non_null(l);
    atx_net_destroy(&l);
    assert_null(l);
    test_reset();
}

static void
test_atx_net_receive_protocol_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    test_init();
    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_protocol_error_serial(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char sid[SID_LEN + 1];
    memset(sid, SID_LEN + 1, 'A');
    zmsg_t* m = helpers_create_message_mem(
        6, "rid", 3, "\x0", 1, "\x0", 1, sid, SID_LEN + 1, "pid", 3, "site", 4);

    test_init();
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    atx_net_s* l = atx_net_create(&callbacks, &pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_protocol_error_router(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char rid[RID_LEN + 1];
    memset(rid, SID_LEN + 1, 'A');
    zmsg_t* m = helpers_create_message_mem(
        6, rid, RID_LEN + 1, "\x0", 1, "\x0", 1, "sid", 3, "pid", 3, "site", 4);

    test_init();
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    atx_net_s* l = atx_net_create(&callbacks, &pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_sid = "serial";
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("rid00", serial, "product", "site");

    test_init();

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_set_incoming((0x01));
    spy_sys_set_tick(100);

    // Receive a heartbeat
    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    device_s** d = atx_net_device(l, serial);
    assert_non_null(d);
    assert_int_equal(atx_net_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 4);
    assert_memory_equal(device_router(*d)->id, "rid0", 4);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    atx_net_poll(l, 5);
    assert_non_null(d);
    assert_int_equal(atx_net_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 5);
    assert_memory_equal(device_router(*d)->id, "rid00", 5);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 100);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_heartbeat_ok_insert_device(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_sid = "serial";
    const char* expect_query = "SELECT EXISTS(SELECT 1 FROM devices WHERE "
                               "device_id=\"serial\" LIMIT 1);";
    const char* expect_insert =
        "INSERT INTO "
        "devices(device_id,product,prj_version,atx_version,web_version,mac) "
        "VALUES(\"serial\",\"LINQ2\",\"2.00.00\",\"2.00.00\",\"2.00.00\","
        "\"00:00:00:AA:BB:CC\");";
    const char* response = "{\"about\":{"
                           "\"sid\":\"serial\","
                           "\"product\":\"LINQ2\","
                           "\"prjVersion\":\"2.00.00\","
                           "\"atxVersion\":\"2.00.00\","
                           "\"webVersion\":\"2.00.00\","
                           "\"mac\":\"00:00:00:AA:BB:CC\""
                           "}}";
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* about = helpers_make_response("rid0", serial, 0, response);
    outgoing_statement* statement = NULL;

    test_init();

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&about);
    czmq_spy_poll_set_incoming((0x01));

    // Database query responses
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(0);

    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    sqlite_spy_outgoing_statement_flush();

    // Receive a heartbeat (Request about)
    atx_net_poll(l, 5);
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_query) + 1, statement->len);
    assert_memory_equal(expect_query, statement->data, statement->len);
    atx_net_free(statement);
    // TODO measure outgoing czmq packet about request

    // Receive about response, and we insert device into database
    atx_net_poll(l, 5);
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_insert) + 1, statement->len);
    assert_memory_equal(expect_insert, statement->data, statement->len);
    atx_net_free(statement);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_heartbeat_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    zmsg_t* m = helpers_create_message_mem(
        4, "router", 6, "\x0", 1, "\x0", 1, "product", 7);

    test_init();

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_alert_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* sid = expect_sid = "sid";
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");

    test_init();

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    pass = false;
    atx_net_poll(l, 5);

    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_alert_insert(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* sid = expect_sid = "sid";
    const char* expect_keys =
        "INSERT INTO "
        "alerts(alert_id,who,what,site_id,time,mesg,device_id)";
    const char* expect_values =
        "VALUES(\"\",\"TestUser\",\"TestAlert\",\"Altronix Site "
        "ID\",\"1\",\"Test Alert Message\",\"sid\");";
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");
    outgoing_statement* statement;

    test_init();

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    sqlite_spy_outgoing_statement_flush();
    pass = false;

    atx_net_poll(l, 5);
    assert_true(pass);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_memory_equal(expect_keys, statement->data, strlen(expect_keys));
    ((void)expect_values); // TODO the uuid is random each test so we can't
                           // compare. (Mocking uuid is challenging)
    atx_net_free(statement);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_alert_error_short(void** context_p)
{
    ((void)context_p);
}

static void
on_response_ok(void* pass, int err, const char* data, device_s** d)
{
    *(bool*)pass = true;
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(device_serial(*d), "serial");
}

static void
test_atx_net_receive_response_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_sid = "serial";
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&r);
    czmq_spy_poll_set_incoming((0x01));

    // Receive heartbeat (add device to linq)
    // Send a get request
    // receive get response
    // make sure callback is as expect
    atx_net_s* l = atx_net_create(&callbacks, (void*)&pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    atx_net_send_get(l, serial, "/ATX/test", on_response_ok, &pass);
    atx_net_poll(l, 5);
    assert_true(pass);

    atx_net_destroy(&l);
    test_reset();
}

static void
on_response_error_timeout(void* pass, int err, const char* data, device_s** d)
{
    *((bool*)pass) = true;
    assert_string_equal(device_serial(*d), "serial");
    assert_int_equal(err, LINQ_ERROR_TIMEOUT);
    assert_string_equal(data, "{\"error\":\"timeout\"}");
}

static void
test_atx_net_receive_response_error_timeout(void** context_p)
{
    ((void)context_p);

    bool pass = false, response_pass = false;
    const char* serial = expect_sid = "serial";
    device_s** d;
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    // Receive a new device @t=0
    spy_sys_set_tick(0);
    atx_net_s* l = atx_net_create(&callbacks, &pass);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    d = atx_net_device(l, serial);
    device_send_get(*d, "/ATX/test", on_response_error_timeout, &response_pass);
    assert_int_equal(device_request_pending_count(*d), 1);

    // Still waiting for response @t=9999
    spy_sys_set_tick(9999);
    czmq_spy_poll_set_incoming((0x00));
    atx_net_poll(l, 5);
    assert_false(response_pass);
    assert_int_equal(device_request_pending_count(*d), 1);

    // Timeout callback happens @t=10000
    spy_sys_set_tick(10000);
    atx_net_poll(l, 5);
    assert_true(response_pass);
    assert_int_equal(device_request_pending_count(*d), 0);

    // Response is resolved but there is no more request pending
    czmq_spy_poll_set_incoming((0x01));
    czmq_spy_mesg_push_incoming(&r);
    atx_net_poll(l, 5);

    atx_net_destroy(&l);
    test_reset();
}

static void
on_response_error_codes(void* pass, int err, const char* data, device_s** d)
{
    char expect[32];
    snprintf(expect, sizeof(expect), "{\"error\":%d}", expect_error);
    assert_string_equal(device_serial(*d), "serial");
    assert_string_equal(data, expect);
    assert_int_equal(err, expect_error);
    *((bool*)pass) = true;
}

static void
test_atx_net_receive_response_error_codes(void** context_p)
{
    ((void)context_p);

    bool pass = false;
    const char* serial = expect_sid = "serial";
    device_s** d;

    int codes[] = { 0, 400, 403, 404, 500 };
    for (int i = 0; i < 5; i++) {
        // Setup incoming network (1st poll heartbeat, 2nd poll response)
        test_init();
        sqlite_spy_step_return_push(SQLITE_ROW);
        sqlite_spy_column_int_return_push(1);

        char data[32];
        snprintf(data, sizeof(data), "{\"error\":%d}", codes[i]);
        expect_error = codes[i];
        zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
        zmsg_t* r = helpers_make_response("rid0", serial, codes[i], data);
        czmq_spy_mesg_push_incoming(&hb);
        czmq_spy_mesg_push_incoming(&r);
        czmq_spy_poll_set_incoming((0x01));

        // Setup code under test
        atx_net_s* l = atx_net_create(NULL, NULL);
        atx_net_listen(l, "tcp://*:32820");
        atx_net_poll(l, 0);
        d = atx_net_device(l, serial);
        assert_non_null(d);

        // Start test
        device_send_get(*d, "/ATX/test", on_response_error_codes, &pass);
        assert_int_equal(device_request_pending_count(*d), 1);
        atx_net_poll(l, 0);

        // Measure test
        assert_true(pass);

        // Cleanup test
        atx_net_destroy(&l);
        test_reset();
    }
}

static void
on_response_error_504(void* pass, int err, const char* data, device_s** d)
{
    ((void)d);
    assert_string_equal(data, "{\"error\":504}");
    assert_int_equal(err, LINQ_ERROR_504);
    *((bool*)pass) = true;
}

static void
test_atx_net_receive_response_error_504(void** context_p)
{
    // Same as receive_response_ok_504 accept we add an extra 504 to incoming
    ((void)context_p);
    bool pass = false;
    uint32_t t = 0;
    const char* serial = expect_sid = "serial";
    device_s** d;
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* incoming[LINQ_NETW_MAX_RETRY + 1] = {
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}")
    };
    zmsg_t* outgoing = NULL;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    // Setup code under test
    atx_net_s* l = atx_net_create(&callbacks, &pass);
    atx_net_listen(l, "tcp://*:32820");

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));
    atx_net_poll(l, 0);
    d = atx_net_device(l, serial);
    assert_non_null(d);

    // Start test @t=0
    device_send_get(*d, "/ATX/test", on_response_error_504, &pass);
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    zmsg_destroy(&outgoing);

    for (int i = 0; i < LINQ_NETW_MAX_RETRY; i++) {
        // incoming 504
        czmq_spy_mesg_push_incoming(&incoming[i]);
        czmq_spy_poll_set_incoming((0x01));
        atx_net_poll(l, 0);

        // @t=retry-1, make sure do not send request
        t += LINQ_NETW_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        atx_net_poll(l, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_null(outgoing);

        // @t=retry, send request again, (incoming 504 #2)
        t++;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        atx_net_poll(l, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing); // TODO measure outgoing packets
        zmsg_destroy(&outgoing);
    }

    // Send the final amount of 504's we're willing to tollorate
    czmq_spy_mesg_push_incoming(&incoming[LINQ_NETW_MAX_RETRY]);
    czmq_spy_poll_set_incoming(0x01);
    atx_net_poll(l, 0);

    assert_true(pass);
    atx_net_destroy(&l);
    test_reset();
}

static void
on_response_ok_504(void* pass, int err, const char* data, device_s** d)
{
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(device_serial(*d), "serial");
    *(bool*)pass = true;
}

static void
test_atx_net_receive_response_ok_504(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    uint32_t t = 0;
    const char* serial = expect_sid = "serial";
    device_s** d;
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* ok = helpers_make_response("rid0", serial, 0, "{\"test\":1}");
    zmsg_t* incoming[LINQ_NETW_MAX_RETRY] = {
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}"),
        helpers_make_response("rid0", serial, 504, "{\"error\":504}")
    };
    zmsg_t* outgoing = NULL;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    // Setup code under test
    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");

    // Receive a new device @t=0
    spy_sys_set_tick(t);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));
    atx_net_poll(l, 0);
    d = atx_net_device(l, serial);
    assert_non_null(d);

    // Start test @t=0
    device_send_get(*d, "/ATX/test", on_response_ok_504, &pass);
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    zmsg_destroy(&outgoing);

    for (int i = 0; i < LINQ_NETW_MAX_RETRY; i++) {
        // incoming 504
        czmq_spy_mesg_push_incoming(&incoming[i]);
        czmq_spy_poll_set_incoming((0x01));
        atx_net_poll(l, 0);

        // @t=retry-1, make sure do not send request
        t += LINQ_NETW_RETRY_TIMEOUT - 1;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        atx_net_poll(l, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_null(outgoing);

        // @t=retry, send request again, (incoming 504 #2)
        t++;
        spy_sys_set_tick(t);
        czmq_spy_poll_set_incoming((0x00));
        atx_net_poll(l, 0);
        outgoing = czmq_spy_mesg_pop_outgoing();
        assert_non_null(outgoing); // TODO measure outgoing packets
        zmsg_destroy(&outgoing);
        assert_false(pass);
    }

    czmq_spy_mesg_push_incoming(&ok);
    czmq_spy_poll_set_incoming(0x01);
    atx_net_poll(l, 0);

    assert_true(pass);
    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_hello(void** context_p)
{
    ((void)context_p);

    test_init();

    zmsg_t* m = helpers_make_hello("router", "node");
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));
    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");

    assert_int_equal(atx_net_node_count(l), 0);
    atx_net_poll(l, 5);
    assert_int_equal(atx_net_node_count(l), 1);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_receive_hello_double_id(void** context_p)
{
    ((void)context_p);
    zmsg_t* m0 = helpers_make_hello("router", "node");
    zmsg_t* m1 = helpers_make_hello("router", "node");

    test_init();

    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");
    assert_int_equal(atx_net_node_count(l), 0);
    atx_net_poll(l, 5);
    assert_int_equal(atx_net_node_count(l), 1);
    atx_net_poll(l, 5);
    assert_int_equal(atx_net_node_count(l), 1);

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_broadcast_heartbeat_receive(void** context_p)
{
    ((void)context_p);
    zmsg_t* hb = helpers_make_heartbeat(NULL, "serial", "product", "site");

    test_init();

    atx_net_s* linq = atx_net_create(NULL, NULL);

    atx_net_connect(linq, "ipc:///123");

    // TODO - this heartbeat comes from a dealer socket
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));
    atx_net_poll(linq, 5);

    atx_net_destroy(&linq);
    test_reset();
}

static void
test_atx_net_broadcast_heartbeat(void** context_p)
{
    ((void)context_p);

    zmsg_t* hb = helpers_make_heartbeat("rid0", "serial", "product", "site");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    // Client sends hello to server, device sends heartbeat to server
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5); // receive hello
    atx_net_poll(l, 5); // recieve hello
    atx_net_poll(l, 5); // receive heartbeat

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
        assert_memory_equal(zframe_data(typ), "\x0", 1);
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

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_broadcast_alert(void** context_p)
{
    ((void)context_p);

    zmsg_t* hb = helpers_make_heartbeat("rid0", "sid", "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", "sid", "pid");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    // device sends heartbeat to server, two clients connect, device sends alert
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5); // receive heartbeat
    atx_net_poll(l, 5); // receive hello
    atx_net_poll(l, 5); // recieve hello
    atx_net_poll(l, 5); // receive alert

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
        assert_memory_equal(zframe_data(typ), "\x3", 1);
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

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_forward_request(void** context_p)
{
    ((void)context_p);
    zmsg_t *hb, *hello, *request, *response, *outgoing;
    zframe_t *rid, *ver, *typ, *sid, *url, *err, *dat;
    hb = helpers_make_heartbeat("router-d", "device123", "pid", "site");
    hello = helpers_make_hello("router-c", "client123");
    request = helpers_make_request("router-c", "device123", "GET /hello", NULL);
    response = helpers_make_response("router-d", "device123", 0, "world");

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);       // device heartbeat
    czmq_spy_mesg_push_incoming(&hello);    // remote client hello
    czmq_spy_mesg_push_incoming(&request);  // remote client request
    czmq_spy_mesg_push_incoming(&response); // device response
    czmq_spy_poll_set_incoming((0x01));

    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_listen(l, "tcp://*:32820");
    atx_net_poll(l, 5);
    atx_net_poll(l, 5);
    atx_net_poll(l, 5);
    atx_net_poll(l, 5);

    // First outgoing message is to the device
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    assert_int_equal(zmsg_size(outgoing), 4);
    rid = zmsg_pop(outgoing);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    url = zmsg_pop(outgoing);

    assert_memory_equal(zframe_data(rid), "router-d", 8);
    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x1", 1);
    assert_memory_equal(zframe_data(url), "GET /hello", 10);
    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
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

    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_forward_client_request(void** context_p)
{
    ((void)context_p);
    zframe_t *ver, *typ, *sid, *url;
    zmsg_t* hb = helpers_make_heartbeat(NULL, "device123", "pid", "site");
    zmsg_t* outgoing = NULL;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming(0x01);

    atx_net_s* l = atx_net_create(NULL, NULL);
    atx_net_connect(l, "ipc:///test");

    atx_net_poll(l, 5); // add a device

    atx_net_send_get(l, "device123", "/ATX/hello", NULL, NULL);
    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    zmsg_destroy(&outgoing); // delete outgoing hello frames
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
    atx_net_destroy(&l);
    test_reset();
}

static void
test_atx_net_connect(void** context_p)
{
    ((void)context_p);
    atx_net_socket s;

    test_init();

    atx_net_s* linq = atx_net_create(NULL, NULL);

    s = atx_net_connect(linq, "ipc:///filex");
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

    atx_net_destroy(&linq);
    test_reset();
}

static void
test_atx_net_close_router(void** context_p)
{
    ((void)context_p);

    test_init();

    atx_net_s* linq = atx_net_create(NULL, NULL);
    atx_net_socket l0 = atx_net_listen(linq, "tcp://1.2.3.4:8080");
    atx_net_socket l1 = atx_net_listen(linq, "tcp://5.6.7.8:8080");
    atx_net_socket c0 = atx_net_connect(linq, "tcp://11.22.33.44:8888");
    atx_net_socket c1 = atx_net_connect(linq, "tcp://55.66.77.88:8888");
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
    atx_net_poll(linq, 5);
    atx_net_poll(linq, 5);
    assert_int_equal(atx_net_device_count(linq), 8);
    atx_net_close(linq, l0);
    assert_int_equal(atx_net_device_count(linq), 6);
    atx_net_close(linq, l1);
    assert_int_equal(atx_net_device_count(linq), 4);
    atx_net_close(linq, c0);
    assert_int_equal(atx_net_device_count(linq), 2);
    assert_int_equal(atx_net_node_count(linq), 1);
    atx_net_close(linq, c1);
    assert_int_equal(atx_net_device_count(linq), 0);
    assert_int_equal(atx_net_node_count(linq), 0);

    atx_net_destroy(&linq);
    test_reset();
}

static void
test_atx_net_devices_callback(void* context, const char* sid, const char* pid)
{
    uint32_t *mask = context, idx = 0;
    assert_memory_equal(sid, "dev", 3);
    assert_memory_equal(pid, "pid", 3);
    idx = atoi(&sid[3]);
    *mask |= (0x01 << idx);
    assert_int_equal(idx, atoi(&pid[3]));
}

static void
test_atx_net_devices_foreach(void** context_p)
{
    ((void)context_p);

    test_init();

    atx_net_s* linq = atx_net_create(NULL, NULL);
    atx_net_listen(linq, "tcp://1.2.3.4:8080");
    atx_net_listen(linq, "tcp://5.6.7.8:8080");
    atx_net_connect(linq, "tcp://11.22.33.44:8888");
    atx_net_connect(linq, "tcp://55.66.77.88:8888");
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
    atx_net_poll(linq, 5);
    atx_net_poll(linq, 5);
    assert_int_equal(atx_net_device_count(linq), 8);

    uint32_t mask = 0x00;
    atx_net_devices_foreach(linq, test_atx_net_devices_callback, &mask);
    assert_int_equal(mask, 0b11111111);

    atx_net_destroy(&linq);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_atx_net_create),
        cmocka_unit_test(test_atx_net_receive_protocol_error_short),
        cmocka_unit_test(test_atx_net_receive_protocol_error_serial),
        cmocka_unit_test(test_atx_net_receive_protocol_error_router),
        cmocka_unit_test(test_atx_net_receive_heartbeat_ok),
        cmocka_unit_test(test_atx_net_receive_heartbeat_ok_insert_device),
        cmocka_unit_test(test_atx_net_receive_heartbeat_error_short),
        cmocka_unit_test(test_atx_net_receive_alert_ok),
        cmocka_unit_test(test_atx_net_receive_alert_insert),
        cmocka_unit_test(test_atx_net_receive_alert_error_short),
        cmocka_unit_test(test_atx_net_receive_response_ok),
        cmocka_unit_test(test_atx_net_receive_response_error_timeout),
        cmocka_unit_test(test_atx_net_receive_response_error_codes),
        cmocka_unit_test(test_atx_net_receive_response_error_504),
        cmocka_unit_test(test_atx_net_receive_response_ok_504),
        cmocka_unit_test(test_atx_net_receive_hello),
        cmocka_unit_test(test_atx_net_receive_hello_double_id),
        cmocka_unit_test(test_atx_net_broadcast_heartbeat),
        cmocka_unit_test(test_atx_net_broadcast_heartbeat_receive),
        cmocka_unit_test(test_atx_net_broadcast_alert),
        cmocka_unit_test(test_atx_net_forward_request),
        cmocka_unit_test(test_atx_net_forward_client_request),
        cmocka_unit_test(test_atx_net_connect),
        cmocka_unit_test(test_atx_net_close_router),
        cmocka_unit_test(test_atx_net_devices_foreach)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
