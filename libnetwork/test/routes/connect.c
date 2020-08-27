#include "connect.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "netw.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_connect(void** context_p)
{
    const char* req = "/api/v1/connect";
    const char* body = "{\"endpoint\":\"tcp://1.2.3.4:3333\"}";
    const char* expect = "{\"error\":\"Ok\"}";
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };

    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_outgoing_statement_flush();
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    assert_int_equal(netw_node_count(test->net), 0);

    mongoose_spy_event_request_push(UNSAFE_TOKEN, "POST", req, body);
    for (int i = 0; i < 4; i++) netw_poll(test->net, -1);
    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, expect, strlen(expect));
    mock_mongoose_response_destroy(&response);

    assert_int_equal(netw_node_count(test->net), 1);

    test_reset(&test);
}
