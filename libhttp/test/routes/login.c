#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_utils.h"
#include "netw.h"
#include "routes.h"

#include "main.h"

void
test_route_login_ok(void** context_p)
{
    ((void)context_p);
    int l;
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body = "{"
                       "\"user\":\"" UNSAFE_USER "\","
                       "\"pass\":\"" UNSAFE_PASS "\""
                       "}";

    helpers_test_context_s* test = test_init(&config);
    spy_sys_set_unix(UNSAFE_IAT); // unsafe "issued at"

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push(UNSAFE_UUID);
    sqlite_spy_column_text_return_push(UNSAFE_USER);
    sqlite_spy_column_text_return_push(UNSAFE_HASH);
    sqlite_spy_column_text_return_push(UNSAFE_SALT);

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "POST", "/api/v1/public/login", body);
    for (int i = 0; i < 4; i++) http_poll(&test->http, -1);

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
    test_reset(&test);
}

void
test_route_login_bad_pass(void** context_p)
{
    ((void)context_p);
}
