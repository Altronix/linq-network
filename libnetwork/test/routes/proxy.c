#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "netw.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_proxy_get(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };

    const char* serial = "serial1234";
    zmsg_t* outgoing;
    zframe_t *rid, *ver, *typ, *url;

    helpers_test_context_s* test = test_init(&config);
    helpers_add_device(test, serial, "rid0", "product", "site");

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/api/v1/proxy/serial1234/ATX/about", NULL);
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) netw_poll(test->net, -1);

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

    test_reset(&test);
}

void
test_route_proxy_post(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };

    const char* serial = "serial1234";
    zmsg_t* outgoing;
    zframe_t *rid, *ver, *typ, *url, *dat;

    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    helpers_add_device(test, serial, "rid0", "product", "site");
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN,
        "POST",
        "/api/v1/proxy/serial1234/ATX/about",
        "{\"test\":\"data\"}");
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) netw_poll(test->net, -1);

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

    test_reset(&test);
}

void
test_route_proxy_404(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    netw_poll(test->net, 5);
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN,
        "POST",
        "/api/v1/proxy/serial1234/ATX/about",
        "{\"test\":\"data\"}");
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) netw_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, "{\"error\":\"not found\"}", 21);
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_proxy_400_too_short(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    netw_poll(test->net, 5);
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/api/v1/proxy/1234", NULL);
    czmq_spy_poll_set_incoming((0x00));
    for (int i = 0; i < 4; i++) netw_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, "{\"error\":\"Bad request\"}", 23);
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}
