#include "altronix/linq_netw.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

static void
test_init()
{
    mongoose_spy_init();
    sqlite_spy_init();
    sqlite_spy_step_return_push(SQLITE_DONE); // PRAGMA
    sqlite_spy_step_return_push(SQLITE_ROW);  // device database OK
    sqlite_spy_step_return_push(SQLITE_ROW);  // device database OK
}

static void
test_reset()
{
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

static void
test_route_devices(void** context_p)
{
    ((void)context_p);
    const char* body_expect =
        "{"
        "\"devices\":{"
        "\"A\":{\"product\":\"A\",\"prj_version\":\"A\",\"atx_version\":\"A\"},"
        "\"A\":{\"product\":\"A\",\"prj_version\":\"A\",\"atx_version\":\"A\"},"
        "\"A\":{\"product\":\"A\",\"prj_version\":\"A\",\"atx_version\":\"A\"}"
        "}}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_set("A");
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/devices", NULL);
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_route_devices_response_too_large(void** context_p)
{
    ((void)context_p);
    const char* body_expect = "{\"error\":\"Response too large\"}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    for (int i = 0; i < 10000; i++) sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_set("A");
    mongoose_spy_event_request_push(
        "", "GET", "/api/v1/linq-lite/devices", NULL);
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

static void
test_route_devices_response_get_only(void** context_p)
{
    ((void)context_p);
    const char* body_expect = "{\"error\":\"Bad request\"}";

    test_init();
    linq_netw_s* l = linq_netw_create(NULL, NULL);
    linq_netw_listen(l, "tcp://*:32820");
    linq_netw_listen(l, "http://*:8000");
    sqlite_spy_outgoing_statement_flush();

    sqlite_spy_column_text_return_set("A");
    mongoose_spy_event_request_push(
        "", "POST", "/api/v1/linq-lite/devices", "{\"blah\":\"blah\"}");
    for (int i = 0; i < 4; i++) linq_netw_poll(l, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    // TODO check mock_mongoose and post request if working
    // assert_memory_equal(response->body, body_expect, strlen(body_expect));
    mock_mongoose_response_destroy(&response);

    linq_netw_destroy(&l);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_route_devices),
        cmocka_unit_test(test_route_devices_response_too_large),
        cmocka_unit_test(test_route_devices_response_get_only),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
