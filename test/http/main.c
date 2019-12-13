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

static int
test_http_hello_route(http_request_s* request, E_HTTP_METHOD method)
{
    assert_int_equal(method, HTTP_GET);
    assert_int_equal(request->body.len, 0);
    assert_null(request->body.p);
    return 200;
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
test_http_simple_route(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();

    http_s http;
    http_init(&http);

    mongoose_spy_event_request_push("admin:admin", "GET", "/hello", NULL);

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
                                        cmocka_unit_test(test_http_simple_route)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
