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
    char token_expect[2048];
    l = snprintf(
        token_expect, sizeof(token_expect), "{\"token\":\"%s\"}", UNSAFE_TOKEN);

    helpers_test_context_s* test = test_init(&config);
    spy_sys_set_unix(UNSAFE_IAT); // unsafe "issued at"

    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_text_return_push(UNSAFE_UUID);
    sqlite_spy_column_text_return_push(UNSAFE_USER);
    sqlite_spy_column_text_return_push(UNSAFE_HASH);
    sqlite_spy_column_text_return_push(UNSAFE_SALT);

    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "POST", "/api/v1/public/login", body);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);

    mongoose_parser_context* response = mongoose_spy_response_pop();
    mock_mongoose_response_destroy(&response);

    test_reset(&test);
}

void
test_route_login_bad_pass(void** context_p)
{
    ((void)context_p);
}
