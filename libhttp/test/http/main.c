// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "http.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "netw.h"

#include <cmocka.h>
#include <setjmp.h>

static void
test_http_create(void** c_p)
{
    ((void)c_p);
    mongoose_spy_init();
    sqlite_spy_init();
    netw_s* l = netw_create(NULL, NULL);
    http_s http;
    http_init(&http, NULL);
    http_deinit(&http);
    netw_destroy(&l);
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

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
    *((bool*)ctx->context) = true;
    http_printf_json(ctx->curr_connection, 200, "{\"hello\":\"world\"}");
}

static void
test_http_simple_get(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();
    sqlite_spy_init();
    bool pass = false;

    // Init http
    netw_s* l = netw_create(NULL, NULL);
    netw_listen(l, "80");
    netw_use(l, "/hello", test_http_hello_route, &pass);

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    // Generate some events
    mongoose_spy_event_request_push(UNSAFE_TOKEN, "GET", "/hello", NULL);
    while (netw_poll(l, 0)) {};

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_int_equal(response->content_length, 17);
    assert_memory_equal(response->body, "{\"hello\":\"world\"}", 17);
    mock_mongoose_response_destroy(&response);

    assert_true(pass);
    netw_destroy(&l);
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

static void
test_http_query_route(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    assert_int_equal(meth, HTTP_METHOD_GET);
    assert_int_equal(l, 0);
    assert_null(body);
    *((bool*)ctx->context) = true;
    const char* query = "BOO";
    uint32_t len = 3;
    http_parse_query_str(ctx, "notfound", &query, &len);
    assert_null(query);
    assert_int_equal(len, 0);
    http_parse_query_str(ctx, "a", &query, &len);
    assert_non_null(query);
    assert_int_equal(len, 1);
    assert_memory_equal(query, "1", 1);
    http_parse_query_str(ctx, "param", &query, &len);
    assert_non_null(query);
    assert_int_equal(len, (sizeof("echo") - 1));
    assert_memory_equal(query, "echo", len);
    http_parse_query_str(ctx, "start", &query, &len);
    assert_non_null(query);
    assert_int_equal(len, (sizeof("22") - 1));
    assert_memory_equal(query, "22", len);
    http_parse_query_str(ctx, "b", &query, &len);
    assert_non_null(query);
    assert_int_equal(len, 1);
    assert_memory_equal(query, "2", 1);
    http_printf_json(ctx->curr_connection, 200, "{\"hello\":\"world\"}");
}

static void
test_http_simple_query(void** context_p)
{
    ((void)context_p);
    database_s db;
    mongoose_spy_init();
    sqlite_spy_init();
    bool pass = false;

    // Init http
    netw_s* l = netw_create(NULL, NULL);
    netw_listen(l, "80");
    netw_use(l, "/hello", test_http_hello_route, &pass);

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    // Generate some events
    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/hello?a=1&param=echo&start=22&b=2", NULL);
    while (netw_poll(l, 0)) {};

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_int_equal(response->content_length, 17);
    assert_memory_equal(response->body, "{\"hello\":\"world\"}", 17);
    mock_mongoose_response_destroy(&response);

    assert_true(pass);
    netw_destroy(&l);
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

static void
test_http_invalid_query_route(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    assert_int_equal(meth, HTTP_METHOD_GET);
    assert_int_equal(l, 0);
    assert_null(body);
    const char* test = "BOO";
    const char* query = test;
    uint32_t len = 3;
    (*((int*)ctx->context))++;

    http_parse_query_str(ctx, "invalid", &query, &len);
    assert_null(query);
    assert_int_equal(len, 0);
    query = test;
    len = 3;
    http_parse_query_str(ctx, "alsoinvalid", &query, &len);
    assert_null(query);
    assert_int_equal(len, 0);
    http_printf_json(ctx->curr_connection, 200, "{\"hello\":\"world\"}");
}

static void
test_http_invalid_query(void** context_p)
{
    ((void)context_p);
    database_s db;
    mongoose_spy_init();
    sqlite_spy_init();
    int pass = 0;

    // Init http
    netw_s* l = netw_create(NULL, NULL);
    netw_listen(l, "80");
    netw_use(l, "/hello", test_http_hello_route, &pass);

    // Mock sqlite database response
    sqlite_spy_step_return_push(SQLITE_ROW); // user exists
    sqlite_spy_column_int_return_push(1);    // user exists

    // Generate some events
    mongoose_spy_event_request_push(
        UNSAFE_TOKEN, "GET", "/hello?invalid&alsoinvalid", NULL);
    while (netw_poll(l, 0)) {};

    mongoose_parser_context* response = mongoose_spy_response_pop();
    assert_non_null(response);
    assert_int_equal(response->content_length, 17);
    assert_memory_equal(response->body, "{\"hello\":\"world\"}", 17);
    mock_mongoose_response_destroy(&response);

    assert_int_equal(pass, 1);
    netw_destroy(&l);
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_http_create),
        cmocka_unit_test(test_http_simple_get),
        cmocka_unit_test(test_http_simple_query),
        cmocka_unit_test(test_http_invalid_query)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
