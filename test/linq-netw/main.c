#include "altronix/linq_netw.h"
#include "device.h"
#include "helpers.h"
#include "linq_netw_internal.h"
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
linq_netw_on_error_fn(
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
linq_netw_on_heartbeat_fn(void* pass, const char* serial, device_s** d)
{
    assert_string_equal(serial, expect_serial);
    assert_string_equal(device_serial(*d), expect_serial);
    *((bool*)pass) = true;
}

static void
linq_netw_on_alert_fn(
    void* pass,
    linq_netw_alert_s* alert,
    linq_netw_email_s* email,
    device_s** d)
{
    assert_string_equal(device_serial(*d), expect_serial);
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

linq_netw_callbacks callbacks = { .err = linq_netw_on_error_fn,
                                  .hb = linq_netw_on_heartbeat_fn,
                                  .alert = linq_netw_on_alert_fn };

static void
test_linq_netw_create(void** context_p)
{
    ((void)context_p);
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    assert_non_null(l);
    linq_netw_destroy(&l);
    assert_null(l);
}

static void
test_linq_netw_receive_protocol_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    zmsg_t* m = helpers_create_message_str(2, "too", "short");

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(&callbacks, (void*)&pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_protocol_error_serial(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char sid[SID_LEN + 1];
    memset(sid, SID_LEN + 1, 'A');
    zmsg_t* m = helpers_create_message_mem(
        6, "rid", 3, "\x0", 1, "\x0", 1, sid, SID_LEN + 1, "pid", 3, "site", 4);

    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    linq_netw_s* l = linq_netw_create(&callbacks, &pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_protocol_error_router(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    char rid[RID_LEN + 1];
    memset(rid, SID_LEN + 1, 'A');
    zmsg_t* m = helpers_create_message_mem(
        6, rid, RID_LEN + 1, "\x0", 1, "\x0", 1, "sid", 3, "pid", 3, "site", 4);

    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    expect_error = LINQ_ERROR_PROTOCOL;

    linq_netw_s* l = linq_netw_create(&callbacks, &pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_heartbeat_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_serial = "serial";
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* hb1 = helpers_make_heartbeat("rid00", serial, "product", "site");

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_set_incoming((0x01));
    spy_sys_set_tick(100);

    // Receive a heartbeat
    linq_netw_s* l = linq_netw_create(&callbacks, (void*)&pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);
    device_s** d = linq_netw_device(l, serial);
    assert_non_null(d);
    assert_int_equal(linq_netw_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 4);
    assert_memory_equal(device_router(*d)->id, "rid0", 4);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 0);

    // Receive a second heartbeat , update router id and last seen
    spy_sys_set_tick(200);
    linq_netw_poll(l, 5);
    assert_non_null(d);
    assert_int_equal(linq_netw_device_count(l), 1);
    assert_int_equal(device_router(*d)->sz, 5);
    assert_memory_equal(device_router(*d)->id, "rid00", 5);
    assert_string_equal(device_serial(*d), serial);
    assert_string_equal(device_type(*d), "product");
    assert_int_equal(device_uptime(*d), 100);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_heartbeat_error_short(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    zmsg_t* m = helpers_create_message_mem(
        4, "router", 6, "\x0", 1, "\x0", 1, "product", 7);

    expect_error = LINQ_ERROR_PROTOCOL;
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(&callbacks, (void*)&pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_alert_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* sid = expect_serial = "sid";
    zmsg_t* hb = helpers_make_heartbeat("rid", sid, "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", sid, "pid");

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(&callbacks, (void*)&pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);
    pass = false;
    linq_netw_poll(l, 5);

    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_alert_error_short(void** context_p)
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
test_linq_netw_receive_response_ok(void** context_p)
{
    ((void)context_p);
    bool pass = false;
    const char* serial = expect_serial = "serial";
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&r);
    czmq_spy_poll_set_incoming((0x01));

    // Receive heartbeat (add device to linq)
    // Send a get request
    // receive get response
    // make sure callback is as expect
    linq_netw_s* l = linq_netw_create(&callbacks, (void*)&pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);
    linq_netw_device_send_get(l, serial, "/ATX/test", on_response_ok, &pass);
    linq_netw_poll(l, 5);
    assert_true(pass);

    linq_netw_destroy(&l);
    test_reset();
}

static void
on_response_error_timeout(void* pass, int err, const char* data, device_s** d)
{
    ((void)d);
    *((bool*)pass) = true;
    assert_int_equal(err, LINQ_ERROR_TIMEOUT);
    assert_string_equal(data, "{\"error\":\"timeout\"}");
}

static void
test_linq_netw_receive_response_error_timeout(void** context_p)
{
    ((void)context_p);

    bool pass = false, response_pass = false;
    const char* serial = expect_serial = "serial";
    device_s** d;
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", serial, 0, "{\"test\":1}");

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    // Receive a new device @t=0
    spy_sys_set_tick(0);
    linq_netw_s* l = linq_netw_create(&callbacks, &pass);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);
    d = linq_netw_device(l, serial);
    device_send_get(*d, "/ATX/test", on_response_error_timeout, &response_pass);
    assert_int_equal(device_request_pending_count(*d), 1);

    // Still waiting for response @t=9999
    spy_sys_set_tick(9999);
    czmq_spy_poll_set_incoming((0x00));
    linq_netw_poll(l, 5);
    assert_false(response_pass);
    assert_int_equal(device_request_pending_count(*d), 1);

    // Timeout callback happens @t=10000
    spy_sys_set_tick(10000);
    linq_netw_poll(l, 5);
    assert_true(response_pass);
    assert_int_equal(device_request_pending_count(*d), 0);

    // Response is resolved but there is no more request pending
    czmq_spy_poll_set_incoming((0x01));
    czmq_spy_mesg_push_incoming(&r);
    linq_netw_poll(l, 5);

    linq_netw_destroy(&l);
    test_reset();
}

static void
on_response_error_codes(void* pass, int err, const char* data, device_s** d)
{
    ((void)d);
    char expect[32];
    snprintf(expect, sizeof(expect), "{\"error\":%d}", expect_error);
    assert_string_equal(data, expect);
    assert_int_equal(err, expect_error);
    *((bool*)pass) = true;
}

static void
test_linq_netw_receive_response_error_codes(void** context_p)
{
    ((void)context_p);

    bool pass = false;
    const char* serial = expect_serial = "serial";
    device_s** d;

    int codes[] = { 0, 400, 403, 404, 500 };
    for (int i = 0; i < 5; i++) {
        // Setup incoming network (1st poll heartbeat, 2nd poll response)
        char data[32];
        snprintf(data, sizeof(data), "{\"error\":%d}", codes[i]);
        expect_error = codes[i];
        zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "pid", "sid");
        zmsg_t* r = helpers_make_response("rid0", serial, codes[i], data);
        czmq_spy_mesg_push_incoming(&hb);
        czmq_spy_mesg_push_incoming(&r);
        czmq_spy_poll_set_incoming((0x01));

        // Setup code under test
        linq_netw_s* l = linq_netw_create(NULL, NULL);
        linq_netw_listen(l, "tcp://*:32820");
        linq_netw_poll(l, 0);
        d = linq_netw_device(l, serial);
        assert_non_null(d);

        // Start test
        device_send_get(*d, "/ATX/test", on_response_error_codes, &pass);
        assert_int_equal(device_request_pending_count(*d), 1);
        linq_netw_poll(l, 0);

        // Measure test
        assert_true(pass);

        // Cleanup test
        linq_netw_destroy(&l);
        test_reset();
    }
}

static void
on_response_error_504(void* pass, int err, const char* data, device_s** d)
{
    ((void)pass);
    ((void)err);
    ((void)data);
    ((void)d);
    // TODO
}

static void
test_linq_netw_receive_response_error_504(void** context_p)
{
    ((void)context_p);
    // TODO
}

static void
on_response_ok_504(void* pass, int err, const char* data, device_s** d)
{
    ((void)pass);
    ((void)err);
    ((void)data);
    ((void)d);
    // TODO
}

static void
test_linq_netw_receive_response_ok_504(void** context_p)
{
    ((void)context_p);
    // TODO
}

static void
test_linq_netw_receive_hello(void** context_p)
{
    ((void)context_p);
    zmsg_t* m = helpers_make_hello("router", "node");
    czmq_spy_mesg_push_incoming(&m);
    czmq_spy_poll_set_incoming((0x01));
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");

    assert_int_equal(linq_netw_nodes_count(l), 0);
    linq_netw_poll(l, 5);
    assert_int_equal(linq_netw_nodes_count(l), 1);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_receive_hello_double_id(void** context_p)
{
    ((void)context_p);
    zmsg_t* m0 = helpers_make_hello("router", "node");
    zmsg_t* m1 = helpers_make_hello("router", "node");

    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    assert_int_equal(linq_netw_nodes_count(l), 0);
    linq_netw_poll(l, 5);
    assert_int_equal(linq_netw_nodes_count(l), 1);
    linq_netw_poll(l, 5);
    assert_int_equal(linq_netw_nodes_count(l), 1);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_broadcast_heartbeat_receive(void** context_p)
{
    ((void)context_p);
    zmsg_t* hb = helpers_make_heartbeat(NULL, "serial", "product", "site");
    linq_netw_s* linq = linq_netw_create(NULL, NULL);

    linq_netw_connect(linq, "ipc:///123");

    // TODO - this heartbeat comes from a dealer socket
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));
    linq_netw_poll(linq, 5);

    linq_netw_destroy(&linq);
    test_reset();
}

static void
test_linq_netw_broadcast_heartbeat(void** context_p)
{
    ((void)context_p);

    zmsg_t* hb = helpers_make_heartbeat("rid0", "serial", "product", "site");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    // Client sends hello to server, device sends heartbeat to server
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5); // receive hello
    linq_netw_poll(l, 5); // recieve hello
    linq_netw_poll(l, 5); // receive heartbeat

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

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_broadcast_alert(void** context_p)
{
    ((void)context_p);

    zmsg_t* hb = helpers_make_heartbeat("rid0", "sid", "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", "sid", "pid");
    zmsg_t* m0 = helpers_make_hello("client-router0", "node0");
    zmsg_t* m1 = helpers_make_hello("client-router1", "node1");
    zmsg_t* outgoing;

    // device sends heartbeat to server, two clients connect, device sends alert
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&m0);
    czmq_spy_mesg_push_incoming(&m1);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5); // receive heartbeat
    linq_netw_poll(l, 5); // receive hello
    linq_netw_poll(l, 5); // recieve hello
    linq_netw_poll(l, 5); // receive alert

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

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_forward_request(void** context_p)
{
    ((void)context_p);
    zmsg_t *hb, *hello, *request, *response, *outgoing;
    zframe_t *rid, *ver, *typ, *sid, *url, *err, *dat;
    hb = helpers_make_heartbeat("router-d", "device123", "pid", "site");
    hello = helpers_make_hello("router-c", "client123");
    request = helpers_make_request("router-c", "device123", "GET /hello", NULL);
    response = helpers_make_response("router-d", "device123", 0, "world");

    czmq_spy_mesg_push_incoming(&hb);       // device heartbeat
    czmq_spy_mesg_push_incoming(&hello);    // remote client hello
    czmq_spy_mesg_push_incoming(&request);  // remote client request
    czmq_spy_mesg_push_incoming(&response); // device response
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_poll(l, 5);
    linq_netw_poll(l, 5);
    linq_netw_poll(l, 5);
    linq_netw_poll(l, 5);

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

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_forward_client_request(void** context_p)
{
    ((void)context_p);
    zframe_t *ver, *typ, *sid, *url;
    zmsg_t* hb = helpers_make_heartbeat(NULL, "device123", "pid", "site");
    zmsg_t* outgoing = NULL;

    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming(0x01);

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_connect(l, "ipc:///test");

    linq_netw_poll(l, 5); // add a device

    linq_netw_device_send_get(l, "device123", "/ATX/hello", NULL, NULL);
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
    linq_netw_destroy(&l);
    test_reset();
}

static void
test_linq_netw_connect(void** context_p)
{
    ((void)context_p);
    linq_netw_socket s;

    linq_netw_s* linq = linq_netw_create(NULL, NULL);

    s = linq_netw_connect(linq, "ipc:///filex");
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

    linq_netw_destroy(&linq);
    test_reset();
}

static void
test_linq_netw_shutdown(void** context_p)
{
    ((void)context_p);
    linq_netw_s* linq = linq_netw_create(NULL, NULL);
    linq_netw_socket l0 = linq_netw_listen(linq, "tcp://1.2.3.4:8080");
    linq_netw_socket l1 = linq_netw_listen(linq, "tcp://5.6.7.8:8080");
    linq_netw_socket c0 = linq_netw_connect(linq, "tcp://11.22.33.44:8888");
    linq_netw_socket c1 = linq_netw_connect(linq, "tcp://55.66.77.88:8888");
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
    linq_netw_poll(linq, 5);
    linq_netw_poll(linq, 5);
    assert_int_equal(linq_netw_device_count(linq), 8);
    linq_netw_shutdown(linq, l0);
    assert_int_equal(linq_netw_device_count(linq), 6);
    linq_netw_shutdown(linq, l1);
    assert_int_equal(linq_netw_device_count(linq), 4);
    linq_netw_disconnect(linq, c0);
    assert_int_equal(linq_netw_device_count(linq), 2);
    assert_int_equal(linq_netw_nodes_count(linq), 1);
    linq_netw_disconnect(linq, c1);
    assert_int_equal(linq_netw_device_count(linq), 0);
    assert_int_equal(linq_netw_nodes_count(linq), 0);

    linq_netw_destroy(&linq);
    test_reset();
}

static void
test_linq_netw_devices_callback(void* context, const char* sid, const char* pid)
{
    uint32_t *mask = context, idx = 0;
    assert_memory_equal(sid, "dev", 3);
    assert_memory_equal(pid, "pid", 3);
    idx = atoi(&sid[3]);
    *mask |= (0x01 << idx);
    assert_int_equal(idx, atoi(&pid[3]));
}

static void
test_linq_netw_devices_foreach(void** context_p)
{
    ((void)context_p);
    linq_netw_s* linq = linq_netw_create(NULL, NULL);
    linq_netw_listen(linq, "tcp://1.2.3.4:8080");
    linq_netw_listen(linq, "tcp://5.6.7.8:8080");
    linq_netw_connect(linq, "tcp://11.22.33.44:8888");
    linq_netw_connect(linq, "tcp://55.66.77.88:8888");
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
    linq_netw_poll(linq, 5);
    linq_netw_poll(linq, 5);
    assert_int_equal(linq_netw_device_count(linq), 8);

    uint32_t mask = 0x00;
    linq_netw_devices_foreach(linq, test_linq_netw_devices_callback, &mask);
    assert_int_equal(mask, 0b11111111);

    linq_netw_destroy(&linq);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_linq_netw_create),
        cmocka_unit_test(test_linq_netw_receive_protocol_error_short),
        cmocka_unit_test(test_linq_netw_receive_protocol_error_serial),
        cmocka_unit_test(test_linq_netw_receive_protocol_error_router),
        cmocka_unit_test(test_linq_netw_receive_heartbeat_ok),
        cmocka_unit_test(test_linq_netw_receive_heartbeat_error_short),
        cmocka_unit_test(test_linq_netw_receive_alert_ok),
        cmocka_unit_test(test_linq_netw_receive_alert_error_short),
        cmocka_unit_test(test_linq_netw_receive_response_ok),
        cmocka_unit_test(test_linq_netw_receive_response_error_timeout),
        cmocka_unit_test(test_linq_netw_receive_response_error_codes),
        cmocka_unit_test(test_linq_netw_receive_response_error_504),
        cmocka_unit_test(test_linq_netw_receive_response_ok_504),
        cmocka_unit_test(test_linq_netw_receive_hello),
        cmocka_unit_test(test_linq_netw_receive_hello_double_id),
        cmocka_unit_test(test_linq_netw_broadcast_heartbeat),
        cmocka_unit_test(test_linq_netw_broadcast_heartbeat_receive),
        cmocka_unit_test(test_linq_netw_broadcast_alert),
        cmocka_unit_test(test_linq_netw_forward_request),
        cmocka_unit_test(test_linq_netw_forward_client_request),
        cmocka_unit_test(test_linq_netw_connect),
        cmocka_unit_test(test_linq_netw_shutdown),
        cmocka_unit_test(test_linq_netw_devices_foreach)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
