// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_netw.h"
#include "http.h"
#include "linq_netw_internal.h"
#include "mock_mongoose.h"
#include "mongoose.h"

#include <cmocka.h>
#include <setjmp.h>

static void
test_http_hello_route(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    assert_int_equal(meth, HTTP_METHOD_GET);
    assert_int_equal(l, 0);
    assert_null(body);
    http_printf_json(ctx, 200, "{\"hello\":\"world\"}");
}

static void
test_http_create(void** c_p)
{
    ((void)c_p);
    http_s http;
    http_init(&http);
    http_deinit(&http);
}

static void
test_http_simple_get(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();
    bool pass = false;

    // Init http
    http_s http;
    http_init(&http);
    http_listen(&http, "80");
    http_use(&http, "/hello", test_http_hello_route, &pass);

    // Generate some events
    mongoose_spy_event_request_push("admin:admin", "GET", "/hello", NULL);
    while (http_poll(&http, 0)) {};

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_int_equal(response->content_length, 17);
    assert_memory_equal(response->body, "{\"hello\":\"world\"}", 17);
    mock_mongoose_response_destroy(&response);

    http_deinit(&http);
    mongoose_spy_deinit();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { //
                                        cmocka_unit_test(test_http_create),
                                        cmocka_unit_test(test_http_simple_get)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
