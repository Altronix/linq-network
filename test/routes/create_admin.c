#include "altronix/atx_net.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "routes/routes.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_create_admin_ok(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = NULL,
                                     .pass = NULL };
    const char* req_path = "/api/v1/public/create_admin";
    const char* req_body = "{\"user\":\"admin\",\"pass\":\"password1234\"}";
    const char* expect_count = "SELECT COUNT(*) FROM users;";
    const char* expect_insert =
        "INSERT INTO "
        "users(user_id,user,pass,salt,role) "
        "VALUES("
        "\"user_id01234\","
        "\"admin\","
        "\"3EE9F302E6119FA253BA057A2D49D82CDE32A0D39EC502987851366EF2A47921\","
        "\"0123456789ABCDEF\","
        "0);";

    outgoing_statement* statement = NULL;

    // Setup uut
    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_column_int_return_push(0);

    // Simulate http request
    mongoose_spy_event_request_push(UNSAFE_TOKEN, "POST", req_path, req_body);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    // Process request
    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, JERROR_200, strlen(JERROR_200));
    mock_mongoose_response_destroy(&response);

    // Expect checking if user exists in database
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_count) + 1, statement->len);
    assert_memory_equal(expect_count, statement->data, statement->len);
    atx_net_free(statement);

    // Expect user added in database and response OK
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_insert) + 1, statement->len);
    assert_memory_equal(expect_insert, statement->data, statement->len);
    atx_net_free(statement);

    test_reset(&test);
}

void
test_route_create_admin_fail_exists(void** context_p)
{
    // TODO add check to not create admin if a user exists and add new behavior
    // to test helper context create
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* req_path = "/api/v1/public/create_admin";
    const char* req_body = "{\"user\":\"admin\",\"pass\":\"password1234\"}";

    // Setup uut
    helpers_test_context_s* test = test_init(&config);
    sqlite_spy_column_int_return_push(1);

    // Simulate http request
    mongoose_spy_event_request_push(UNSAFE_TOKEN, "POST", req_path, req_body);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    // Process request
    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, JERROR_503, strlen(JERROR_503));
    mock_mongoose_response_destroy(&response);

    // Expect user added in database and response OK

    test_reset(&test);
}
