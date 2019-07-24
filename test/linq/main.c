#include "altronix/linq.h"
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "node.h"
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
linq_on_heartbeat_fn(void* pass, const char* serial, node_s** d)
{
    assert_string_equal(serial, expect_serial);
    assert_string_equal(node_serial(*d), expect_serial);
    *((bool*)pass) = true;
}

static void
linq_on_alert_fn(
    void* pass,
    linq_alert_s* alert,
    linq_email_s* email,
    node_s** d)
{
    assert_string_equal(node_serial(*d), expect_serial);
    assert_string_equal(alert->who, "TestUser");
    assert_string_equal(alert->what, "TestAlert");
    assert_string_equal(alert->where, "Altronix Site ID");
    assert_string_equal(alert->when, "1");
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
    linq_s* l = linq_create(NULL, NULL);
    assert_non_null(l);
    linq_destroy(&l);
    assert_null(l);
}

static void
test_linq_receive_protocol_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    linq_s* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_protocol_error_serial(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char sid[SID_LEN + 1];
    memset(sid, SID_LEN + 1, 'A');
    linq_s* l = linq_create(&callbacks, &pass);
    zmsg_t* m = helpers_create_message_mem(
        6, "rid", 3, "\x0", 1, "\x0", 1, sid, SID_LEN + 1, "pid", 3, "site", 4);

    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_protocol_error_router(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char rid[RID_LEN + 1];
    memset(rid, SID_LEN + 1, 'A');
    linq_s* l = linq_create(&callbacks, &pass);
    zmsg_t* m = helpers_create_message_mem(
        6, rid, RID_LEN + 1, "\x0", 1, "\x0", 1, "sid", 3, "pid", 3, "site", 4);

    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    linq_poll(l);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_serial = "serial";
    linq_s* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("rid00", serial, "product", "site");

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_set_incoming((0x01));
    spy_sys_set_tick(100);

    // Receive a heartbeat
    linq_poll(l);
    node_s** d = linq_device(l, serial);
    assert_non_null(d);
    assert_int_equal(linq_device_count(l), 1);
    assert_int_equal(node_router(*d)->sz, 4);
    assert_memory_equal(node_router(*d)->id, "rid0", 4);
    assert_string_equal(node_serial(*d), serial);
    assert_string_equal(node_type(*d), "product");
    assert_int_equal(node_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    linq_poll(l);
    assert_non_null(d);
    assert_int_equal(linq_device_count(l), 1);
    assert_int_equal(node_router(*d)->sz, 5);
    assert_memory_equal(node_router(*d)->id, "rid00", 5);
    assert_string_equal(node_serial(*d), serial);
    assert_string_equal(node_type(*d), "product");
    assert_int_equal(node_uptime(*d), 100);

    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_heartbeat_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    linq_s* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* m = helpers_create_message_mem(
        4, "router", 6, "\x0", 1, "\x0", 1, "product", 7);

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

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
    linq_s* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

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

static void
on_response_ok(void* pass, int err, const char* data, node_s** d)
{
    *(bool*)pass = true;
    assert_int_equal(err, 0);
    assert_string_equal(data, "{\"test\":1}");
    assert_string_equal(node_serial(*d), "serial");
}

static void
test_linq_receive_response_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_serial = "serial";
    linq_s* l = linq_create(&callbacks, (void*)&pass);
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&r);
    czmq_spy_poll_set_incoming((0x01));

    // Receive heartbeat (add device to linq)
    // Send a get request
    // receive get response
    // make sure callback is as expect
    linq_poll(l);
    linq_node_send_get(l, serial, "/ATX/test", on_response_ok, &pass);
    linq_poll(l);
    assert_true(pass);

    linq_destroy(&l);
    test_reset();
}

static void
on_response_error_timeout(void* pass, int err, const char* data, node_s** d)
{
    ((void)d);
    *((bool*)pass) = true;
    assert_int_equal(err, LINQ_ERROR_TIMEOUT);
    assert_string_equal(data, "{\"error\":\"timeout\"}");
}

static void
test_linq_receive_response_error_timeout(void** context_p)
{
    ((void)context_p);

    bool pass = false, response_pass = false;
    const char* serial = expect_serial = "serial";
    linq_s* l = linq_create(&callbacks, &pass);
    node_s** n;
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    // Receive a new device @t=0
    spy_sys_set_tick(0);
    linq_poll(l);
    n = linq_device(l, serial);
    node_send_get(*n, "/ATX/test", on_response_error_timeout, &response_pass);
    assert_int_equal(node_request_pending_count(*n), 1);

    // Still waiting for response @t=9999
    spy_sys_set_tick(9999);
    czmq_spy_poll_set_incoming((0x00));
    linq_poll(l);
    assert_false(response_pass);
    assert_int_equal(node_request_pending_count(*n), 1);

    // Timeout callback happens @t=10000
    spy_sys_set_tick(10000);
    linq_poll(l);
    assert_true(response_pass);
    assert_int_equal(node_request_pending_count(*n), 0);

    // Response is resolved but there is no more request pending
    czmq_spy_poll_set_incoming((0x01));
    czmq_spy_mesg_push_incoming(&r);
    linq_poll(l);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_hello(void** context_p)
{
    ((void)context_p);
    linq_s* l = linq_create(NULL, NULL);
    zmsg_t* m = helpers_make_hello("router", "node");
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    assert_int_equal(linq_server_count(l), 0);
    linq_poll(l);
    assert_int_equal(linq_server_count(l), 1);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_receive_hello_double_id(void** context_p)
{
    ((void)context_p);
    linq_s* l = linq_create(NULL, NULL);
    zmsg_t* m0 = helpers_make_hello("router", "node");
    zmsg_t* m1 = helpers_make_hello("router", "node");

    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_poll_set_incoming((0x01));

    assert_int_equal(linq_server_count(l), 0);
    linq_poll(l);
    assert_int_equal(linq_server_count(l), 1);
    linq_poll(l);
    assert_int_equal(linq_server_count(l), 1);

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_broadcast_heartbeat(void** context_p)
{
    ((void)context_p);

    linq_s* l = linq_create(NULL, NULL);
    zmsg_t* hb = helpers_make_heartbeat("rid0", "serial", "product", "site");
    zmsg_t* m0 = helpers_make_hello("client-router", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router", "node1");
    zmsg_t* outgoing;

    // Client sends hello to server, device sends heartbeat to server
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    linq_poll(l); // receive hello
    linq_poll(l); // recieve hello
    linq_poll(l); // receive heartbeat

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

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_broadcast_alert(void** context_p)
{
    ((void)context_p);

    linq_s* l = linq_create(NULL, NULL);
    zmsg_t* hb = helpers_make_heartbeat("rid0", "sid", "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", "sid", "pid");
    zmsg_t* m0 = helpers_make_hello("client-router", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router", "node1");
    zmsg_t* outgoing;

    // device sends heartbeat to server, two clients connect, device sends alert
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    linq_poll(l); // receive heartbeat
    linq_poll(l); // receive hello
    linq_poll(l); // recieve hello
    linq_poll(l); // receive alert

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

    linq_destroy(&l);
    test_reset();
}

static void
test_linq_forward_request(void** context_p)
{
    ((void)context_p);
    linq_s* l = linq_create(NULL, NULL);
    zmsg_t* hb = helpers_make_heartbeat("rid0", "sid", "pid", "site");

    czmq_spy_mesg_push_incoming(&hb);
    // TODO push hello
    // TODO push request
    // TODO push response
    // TODO readback outgoing response to the hello router
    czmq_spy_poll_set_incoming((0x01));

    linq_destroy(&l);
    test_reset();
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
        cmocka_unit_test(test_linq_receive_response_ok),
        cmocka_unit_test(test_linq_receive_response_error_timeout),
        cmocka_unit_test(test_linq_receive_hello),
        cmocka_unit_test(test_linq_receive_hello_double_id),
        cmocka_unit_test(test_linq_broadcast_heartbeat),
        cmocka_unit_test(test_linq_broadcast_alert),
        cmocka_unit_test(test_linq_forward_request)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
