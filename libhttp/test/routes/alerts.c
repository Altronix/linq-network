#include "helpers.h"
#include "linq_network.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_alerts(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{"
                              "\"alerts\":["
                              "{"
                              "\"alert_id\":\"aid0\","
                              "\"device_id\":\"did0\","
                              "\"who\":\"who0\","
                              "\"what\":\"what0\","
                              "\"site_id\":\"site_id0\","
                              "\"when\":1234,"
                              "\"mesg\":\"mesg0\""
                              "},"
                              "{"
                              "\"alert_id\":\"aid1\","
                              "\"device_id\":\"did1\","
                              "\"who\":\"who1\","
                              "\"what\":\"what1\","
                              "\"site_id\":\"site_id1\","
                              "\"when\":1234,"
                              "\"mesg\":\"mesg1\""
                              "},"
                              "{"
                              "\"alert_id\":\"aid2\","
                              "\"device_id\":\"did2\","
                              "\"who\":\"who2\","
                              "\"what\":\"what2\","
                              "\"site_id\":\"site_id2\","
                              "\"when\":1234,"
                              "\"mesg\":\"mesg2\""
                              "}"
                              "]}";

    helpers_test_context_s* test = test_init(&config);

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push("aid0");
    sqlite_spy_column_text_return_push("who0");
    sqlite_spy_column_text_return_push("what0");
    sqlite_spy_column_text_return_push("site_id0");
    sqlite_spy_column_int_return_push(1234);
    sqlite_spy_column_text_return_push("mesg0");
    sqlite_spy_column_text_return_push("name0");
    sqlite_spy_column_text_return_push("did0");
    sqlite_spy_column_text_return_push("aid1");
    sqlite_spy_column_text_return_push("who1");
    sqlite_spy_column_text_return_push("what1");
    sqlite_spy_column_text_return_push("site_id1");
    sqlite_spy_column_int_return_push(1234);
    sqlite_spy_column_text_return_push("mesg1");
    sqlite_spy_column_text_return_push("name1");
    sqlite_spy_column_text_return_push("did1");
    sqlite_spy_column_text_return_push("aid2");
    sqlite_spy_column_text_return_push("who2");
    sqlite_spy_column_text_return_push("what2");
    sqlite_spy_column_text_return_push("site_id2");
    sqlite_spy_column_int_return_push(1234);
    sqlite_spy_column_text_return_push("mesg2");
    sqlite_spy_column_text_return_push("name2");
    sqlite_spy_column_text_return_push("did2");
    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/api/v1/alerts", NULL);
    for (int i = 0; i < 4; i++) http_poll(&test->http, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_alerts_response_too_large(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"error\":\"Response too large\"}";

    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    for (int i = 0; i < 10000; i++) sqlite_spy_step_return_push(SQLITE_ROW);
    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/api/v1/alerts", NULL);
    for (int i = 0; i < 4; i++) http_poll(&test->http, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_alerts_response_get_only(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"error\":\"Bad request\"}";

    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "POST", "/api/v1/alerts", "{\"blah\":\"blah\"}");
    for (int i = 0; i < 4; i++) http_poll(&test->http, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_alerts_response_empty(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"alerts\":[]}";

    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_outgoing_statement_flush();

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/api/v1/alerts", NULL);
    for (int i = 0; i < 4; i++) http_poll(&test->http, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}
