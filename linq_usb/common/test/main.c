#include "json.h"
#include "rlp.h"
#include "sys.h"
#include "wire.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
test_wire_print_http_request_alloc(void** context_p)
{
    uint8_t* b = NULL;
    uint32_t l;
    int err;
    rlp* result;
    err = wire_print_http_request_alloc(
        &b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
    assert_int_equal(err, 0);
    result = rlp_parse(b, l);
    assert_non_null(result);
    assert_int_equal(rlp_as_u8(rlp_at(result, 0)), 0);
    assert_int_equal(rlp_as_u8(rlp_at(result, 1)), 0);
    assert_string_equal(rlp_as_str(rlp_at(result, 2)), "POST");
    assert_string_equal(rlp_as_str(rlp_at(result, 3)), "/network");
    assert_string_equal(rlp_as_str(rlp_at(result, 4)), "{\"foo\":\"bar\"}");
    rlp_free(&result);
    free(b);
}

static void
test_wire_print_http_request(void** context_p)
{
    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e;

    rlp* result;
    e = wire_print_http_request(b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
    assert_int_equal(e, 0);
    result = rlp_parse(b, l);
    assert_non_null(result);
    assert_int_equal(rlp_as_u8(rlp_at(result, 0)), 0);
    assert_int_equal(rlp_as_u8(rlp_at(result, 1)), 0);
    assert_string_equal(rlp_as_str(rlp_at(result, 2)), "POST");
    assert_string_equal(rlp_as_str(rlp_at(result, 3)), "/network");
    assert_string_equal(rlp_as_str(rlp_at(result, 4)), "{\"foo\":\"bar\"}");
    rlp_free(&result);
}

static void
test_wire_print_http_request_fmt(void** context_p)
{
    uint8_t* b = NULL;
    uint32_t l;
    int err;
    rlp* result;
    err = wire_print_http_request_alloc(
        &b, &l, "POST", "/network", "{\"foo\":\"%s\"}", "bar");
    assert_int_equal(err, 0);
    result = rlp_parse(b, l);
    assert_non_null(result);
    assert_int_equal(rlp_as_u8(rlp_at(result, 0)), 0);
    assert_int_equal(rlp_as_u8(rlp_at(result, 1)), 0);
    assert_string_equal(rlp_as_str(rlp_at(result, 2)), "POST");
    assert_string_equal(rlp_as_str(rlp_at(result, 3)), "/network");
    assert_string_equal(rlp_as_str(rlp_at(result, 4)), "{\"foo\":\"bar\"}");
    rlp_free(&result);
    free(b);
}

static void
test_wire_print_http_response_alloc(void** context_p)
{
    uint8_t* b = NULL;
    uint32_t l;
    int err;
    rlp* result;
    err = wire_print_http_response_alloc(&b, &l, 200, "{\"error\":\"Ok\"}");
    assert_int_equal(err, 0);
    result = rlp_parse(b, l);
    assert_non_null(result);
    assert_int_equal(rlp_as_u8(rlp_at(result, 0)), 0);
    assert_int_equal(rlp_as_u8(rlp_at(result, 1)), 0);
    assert_int_equal(rlp_as_u16(rlp_at(result, 2)), 200);
    assert_string_equal(rlp_as_str(rlp_at(result, 3)), "{\"error\":\"Ok\"}");
    rlp_free(&result);
    free(b);
}

static void
test_wire_print_http_response(void** context_p)
{
    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e;

    rlp* result;
    e = wire_print_http_response(b, &l, 200, "{\"error\":\"Ok\"}");
    assert_int_equal(e, 0);
    result = rlp_parse(b, l);
    assert_non_null(result);
    assert_int_equal(rlp_as_u8(rlp_at(result, 0)), 0);
    assert_int_equal(rlp_as_u8(rlp_at(result, 1)), 0);
    assert_int_equal(rlp_as_u16(rlp_at(result, 2)), 200);
    assert_string_equal(rlp_as_str(rlp_at(result, 3)), "{\"error\":\"Ok\"}");
    rlp_free(&result);
}

static void
test_wire_parse(void** context_p)
{
    uint8_t b[256];
    uint32_t l = sizeof(b);
    wire_parser_http_request_s r;
    int e;
    e = wire_print_http_request(b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
    assert_int_equal(e, 0);
    wire_parse_http_request(b, l, &r);
    assert_int_equal(0, r.vers);
    assert_int_equal(0, r.type);
    assert_string_equal("POST", r.meth);
    assert_string_equal("/network", r.path);
    assert_string_equal("{\"foo\":\"bar\"}", r.data);
    wire_parser_http_request_free(&r);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_wire_print_http_request_alloc), //
        cmocka_unit_test(test_wire_print_http_request),
        cmocka_unit_test(test_wire_print_http_request_fmt),
        cmocka_unit_test(test_wire_print_http_response_alloc), //
        cmocka_unit_test(test_wire_print_http_response),
        cmocka_unit_test(test_wire_parse)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

