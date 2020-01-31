#include "altronix/atx_net.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_devices(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect =
        "{"
        "\"devices\":{"
        "\"A\":{\"product\":\"B\",\"prj_version\":\"C\",\"atx_version\":\"D\"},"
        "\"A\":{\"product\":\"B\",\"prj_version\":\"C\",\"atx_version\":\"D\"},"
        "\"A\":{\"product\":\"B\",\"prj_version\":\"C\",\"atx_version\":\"D\"}"
        "}}";

    helpers_test_context_s* test = test_init(&config);
    atx_net_listen(test->net, "tcp://*:32820");
    atx_net_listen(test->net, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push("A");
    sqlite_spy_column_text_return_push("B");
    sqlite_spy_column_text_return_push("C");
    sqlite_spy_column_text_return_push("D");
    sqlite_spy_column_text_return_push("A");
    sqlite_spy_column_text_return_push("B");
    sqlite_spy_column_text_return_push("C");
    sqlite_spy_column_text_return_push("D");
    sqlite_spy_column_text_return_push("A");
    sqlite_spy_column_text_return_push("B");
    sqlite_spy_column_text_return_push("C");
    sqlite_spy_column_text_return_push("D");
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/devices", NULL);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_devices_response_too_large(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"error\":\"Response too large\"}";

    helpers_test_context_s* test = test_init(&config);
    atx_net_listen(test->net, "tcp://*:32820");
    atx_net_listen(test->net, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    for (int i = 0; i < 10000; i++) sqlite_spy_step_return_push(SQLITE_ROW);
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/devices", NULL);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_devices_response_get_only(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"error\":\"Bad request\"}";

    helpers_test_context_s* test = test_init(&config);
    atx_net_listen(test->net, "tcp://*:32820");
    atx_net_listen(test->net, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_column_text_return_push("A");
    mongoose_spy_event_request_push(
        "", "POST", "/api/v1/linq-lite/devices", "{\"blah\":\"blah\"}");
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_devices_response_empty(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 0,
                                     .http = 0,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"devices\":{}}";

    helpers_test_context_s* test = test_init(&config);
    atx_net_listen(test->net, "tcp://*:32820");
    atx_net_listen(test->net, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_step_return_push(SQLITE_DONE);
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/devices", NULL);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}
