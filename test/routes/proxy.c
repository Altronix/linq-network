#include "altronix/linq_netw.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_proxy_get(void** context_p)
{
    ((void)context_p);

    const char* serial = "serial1234";
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* outgoing;
    zframe_t *rid, *ver, *typ, *url;

    test_init();
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    linq_netw_poll(l, 5);
    sqlite_spy_outgoing_statement_flush();

    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/proxy/serial1234/ATX/about", NULL);
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    rid = zmsg_pop(outgoing);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    url = zmsg_pop(outgoing);
    assert_memory_equal(zframe_data(rid), "rid0", 4);
    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x1", 1);
    assert_memory_equal(zframe_data(url), "GET /ATX/about", 14);
    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&url);
    zmsg_destroy(&outgoing);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_proxy_post(void** context_p)
{
    ((void)context_p);

    const char* serial = "serial1234";
    zmsg_t* hb = helpers_make_heartbeat("rid0", serial, "product", "site");
    zmsg_t* outgoing;
    zframe_t *rid, *ver, *typ, *url, *dat;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    linq_netw_poll(l, 5);
    sqlite_spy_outgoing_statement_flush();

    mongoose_spy_event_request_push(
        "",
        "POST",
        "/api/v1/linq-lite/proxy/serial1234/ATX/about",
        "{\"test\":\"data\"}");
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    outgoing = czmq_spy_mesg_pop_outgoing();
    assert_non_null(outgoing);
    rid = zmsg_pop(outgoing);
    ver = zmsg_pop(outgoing);
    typ = zmsg_pop(outgoing);
    url = zmsg_pop(outgoing);
    dat = zmsg_pop(outgoing);
    assert_memory_equal(zframe_data(rid), "rid0", 4);
    assert_memory_equal(zframe_data(ver), "\x0", 1);
    assert_memory_equal(zframe_data(typ), "\x1", 1);
    assert_memory_equal(zframe_data(url), "POST /ATX/about", 14);
    assert_memory_equal(zframe_data(dat), "{\"test\":\"data\"}", 15);
    zframe_destroy(&rid);
    zframe_destroy(&ver);
    zframe_destroy(&typ);
    zframe_destroy(&url);
    zframe_destroy(&dat);
    zmsg_destroy(&outgoing);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_proxy_404(void** context_p)
{
    ((void)context_p);

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    linq_netw_poll(l, 5);
    sqlite_spy_outgoing_statement_flush();

    mongoose_spy_event_request_push(
        "",
        "POST",
        "/api/v1/linq-lite/proxy/serial1234/ATX/about",
        "{\"test\":\"data\"}");
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, "{\"error\":\"Device not found\"}", 28);
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_proxy_400_too_short(void** context_p)
{
    ((void)context_p);

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    linq_netw_poll(l, 5);
    sqlite_spy_outgoing_statement_flush();

    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/proxy/1234", NULL);
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, "{\"error\":\"Bad request\"}", 23);
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}