#include "helpers.h"
#include "http_auth_unsafe.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_utils.h"
#include "sys.h"

#define USER UNSAFE_USER
#define PASS UNSAFE_PASS

#include <setjmp.h>

#include <cmocka.h>

void
test_route_login_ok(void** context_p)
{
    ((void)context_p);
    int l;
    const char* body = "{"
                       "\"user\":\"" UNSAFE_USER "\","
                       "\"pass\":\"" UNSAFE_PASS "\""
                       "}";

    helpers_test_init();
    database_s db;
    http_s http;
    database_init(&db);
    http_init(&http, &db);
    http_listen(&http, "8000");
    helpers_test_create_admin(&http, USER, PASS);
    helpers_test_context_flush();
    spy_sys_set_unix(UNSAFE_IAT); // unsafe "issued at"

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push(UNSAFE_UUID);
    sqlite_spy_column_text_return_push(UNSAFE_USER);
    sqlite_spy_column_text_return_push(UNSAFE_HASH);
    sqlite_spy_column_text_return_push(UNSAFE_SALT);

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "POST", "/api/v1/public/login", body);
    for (int i = 0; i < 4; i++) http_poll(&http, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();

    /*
    char buffer[2048];
    jsmntok_t t[20];
    jwt_t* jwt = NULL;
    atx_str token;
    int count, len, err;

    // clang-format off
    count = jsmn_parse_tokens(
        t, 20,
        response->body, response->content_length,
        1,
        "token", &token);
    // clang-format on

    assert_int_equal(count, 1);
    len = snprintf(buffer, sizeof(buffer), "%.*s", token.len, token.p);
    err = jwt_decode(&jwt, buffer, NULL, 0);
    assert_int_equal(err, 0);
    assert_int_equal(UNSAFE_IAT, jwt_get_grant_int(jwt, "iat"));
    assert_int_equal(UNSAFE_IAT + 600, jwt_get_grant_int(jwt, "exp"));
    assert_string_equal(UNSAFE_USER, jwt_get_grant(jwt, "sub"));

    jwt_free(jwt);
    */
    mock_mongoose_response_destroy(&response);
    http_deinit(&http);
    database_deinit(&db);
    helpers_test_reset();
}

void
test_route_login_bad_pass(void** context_p)
{
    ((void)context_p);
}

void create_admin(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body);

void
test_route_create_admin_ok(void** context_p)
{
    ((void)context_p);
    const char* req_path = "/api/v1/public/create_admin";
    const char* req_body = "{"
                           "\"user\":\"" UNSAFE_USER "\","
                           "\"pass\":\"" UNSAFE_PASS "\""
                           "}";
    const char* expect_count = "SELECT COUNT(*) FROM users;";
    const char* expect_insert = "INSERT INTO "
                                "users(user_id,user,pass,salt,role) "
                                "VALUES("
                                "\"user_id01234\","
                                "\"" UNSAFE_USER "\","
                                "\"" UNSAFE_HASH "\","
                                "\"" UNSAFE_SALT "\","
                                "0);";

    outgoing_statement* statement = NULL;

    // Setup uut
    helpers_test_init();
    database_s db;
    http_s http;
    database_init(&db);
    http_init(&http, &db);
    http_listen(&http, "8000");
    helpers_test_context_flush();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(0);

    // Simulate http request
    mongoose_spy_event_request_push(UNSAFE_TOKEN, "POST", req_path, req_body);
    for (int i = 0; i < 4; i++) http_poll(&http, -1);

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
    linq_network_free(statement);

    // Expect user added in database and response OK
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_insert) + 1, statement->len);
    assert_memory_equal(expect_insert, statement->data, statement->len);
    linq_network_free(statement);

    http_deinit(&http);
    database_deinit(&db);
    helpers_test_reset();
}

void
test_route_create_admin_fail_exists(void** context_p)
{
    // TODO add check to not create admin if a user exists and add new behavior
    // to test helper context create
    ((void)context_p);
    const char* req_path = "/api/v1/public/create_admin";
    const char* req_body = "{\"user\":\"admin\",\"pass\":\"password1234\"}";

    // Setup uut
    helpers_test_init();
    database_s db;
    http_s http;
    database_init(&db);
    http_init(&http, &db);
    http_listen(&http, "8000");
    helpers_test_context_flush();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);

    // Simulate http request
    mongoose_spy_event_request_push(UNSAFE_TOKEN, "POST", req_path, req_body);
    for (int i = 0; i < 4; i++) http_poll(&http, -1);

    // Process request
    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_memory_equal(response->body, JERROR_503, strlen(JERROR_503));
    mock_mongoose_response_destroy(&response);

    // Expect user added in database and response OK

    http_deinit(&http);
    database_deinit(&db);
    helpers_test_reset();
}
int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_route_create_admin_fail_exists),
        cmocka_unit_test(test_route_login_ok),
        cmocka_unit_test(test_route_login_ok),
        cmocka_unit_test(test_route_login_bad_pass),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
