#include "altronix/linq_netw.h"
#include "helpers.h"
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
    const char* body_expect = "{"
                              "\"alerts\":["
                              "{"
                              "\"alert_id\":\"aid0\","
                              "\"device_id\":\"did0\","
                              "\"who\":\"who0\","
                              "\"what\":\"what0\","
                              "\"site_id\":\"site_id0\","
                              "\"when\":\"when0\","
                              "\"mesg\":\"mesg0\""
                              "},"
                              "{"
                              "\"alert_id\":\"aid1\","
                              "\"device_id\":\"did1\","
                              "\"who\":\"who1\","
                              "\"what\":\"what1\","
                              "\"site_id\":\"site_id1\","
                              "\"when\":\"when1\","
                              "\"mesg\":\"mesg1\""
                              "},"
                              "{"
                              "\"alert_id\":\"aid2\","
                              "\"device_id\":\"did2\","
                              "\"who\":\"who2\","
                              "\"what\":\"what2\","
                              "\"site_id\":\"site_id2\","
                              "\"when\":\"when2\","
                              "\"mesg\":\"mesg2\""
                              "}"
                              "]}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push("aid0");
    sqlite_spy_column_text_return_push("did0");
    sqlite_spy_column_text_return_push("who0");
    sqlite_spy_column_text_return_push("what0");
    sqlite_spy_column_text_return_push("site_id0");
    sqlite_spy_column_text_return_push("when0");
    sqlite_spy_column_text_return_push("mesg0");
    sqlite_spy_column_text_return_push("aid1");
    sqlite_spy_column_text_return_push("did1");
    sqlite_spy_column_text_return_push("who1");
    sqlite_spy_column_text_return_push("what1");
    sqlite_spy_column_text_return_push("site_id1");
    sqlite_spy_column_text_return_push("when1");
    sqlite_spy_column_text_return_push("mesg1");
    sqlite_spy_column_text_return_push("aid2");
    sqlite_spy_column_text_return_push("did2");
    sqlite_spy_column_text_return_push("who2");
    sqlite_spy_column_text_return_push("what2");
    sqlite_spy_column_text_return_push("site_id2");
    sqlite_spy_column_text_return_push("when2");
    sqlite_spy_column_text_return_push("mesg2");
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/alerts", NULL);
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_alerts_response_too_large(void** context_p)
{
    ((void)context_p);
    const char* body_expect = "{\"error\":\"Response too large\"}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    for (int i = 0; i < 10000; i++) sqlite_spy_step_return_push(SQLITE_ROW);
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/alerts", NULL);
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_alerts_response_get_only(void** context_p)
{
    ((void)context_p);
    const char* body_expect = "{\"error\":\"Bad request\"}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_column_text_return_push("A");
    mongoose_spy_event_request_push(
        "", "POST", "/api/v1/linq-lite/alerts", "{\"blah\":\"blah\"}");
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

void
test_route_alerts_response_empty(void** context_p)
{
    ((void)context_p);
    const char* body_expect = "{\"alerts\":[]}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_step_return_push(SQLITE_DONE);
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/alerts", NULL);
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}
