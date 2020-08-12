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

static void
test_wire_read_size(void** context_p)
{

    static uint8_t self_item[] = { '\x04' };
    static uint8_t short_item[] = { '\x83', 'c', 'a', 't' };
    static uint8_t long_item[] = {
        '\xb8', '\x38', 'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p', 's', 'u',
        'm',    ' ',    'd', 'o', 'l', 'o', 'r', ' ', 's', 'i', 't', ' ',
        'a',    'm',    'e', 't', ',', ' ', 'c', 'o', 'n', 's', 'e', 'c',
        't',    'e',    't', 'u', 'r', ' ', 'a', 'd', 'i', 'p', 'i', 's',
        'i',    'c',    'i', 'n', 'g', ' ', 'e', 'l', 'i', 't'
    };
    static uint8_t empty_list[] = { '\xc0' };
    static uint8_t short_list[] = {
        '\xcc',                //
        '\x83', 'c', 'a', 't', //
        '\x83', 'd', 'o', 'g', //
        '\x83', 'p', 'i', 'g'  //
    };
    static uint8_t long_list[] = {
        '\xf8', '\x3a', '\xb8', '\x38', 'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p',
        's',    'u',    'm',    ' ',    'd', 'o', 'l', 'o', 'r', ' ', 's', 'i',
        't',    ' ',    'a',    'm',    'e', 't', ',', ' ', 'c', 'o', 'n', 's',
        'e',    'c',    't',    'e',    't', 'u', 'r', ' ', 'a', 'd', 'i', 'p',
        'i',    's',    'i',    'c',    'i', 'n', 'g', ' ', 'e', 'l', 'i', 't'
    };
    uint32_t sz;
    int err;
    err = wire_read_sz(&sz, self_item, sizeof(self_item));
    assert_int_equal(sz, sizeof(self_item));
    assert_int_equal(err, 0);
    err = wire_read_sz(&sz, short_item, sizeof(short_item));
    assert_int_equal(sz, sizeof(short_item));
    assert_int_equal(err, 0);
    err = wire_read_sz(&sz, long_item, sizeof(long_item));
    assert_int_equal(sz, sizeof(long_item));
    assert_int_equal(err, 0);

    err = wire_read_sz(&sz, empty_list, sizeof(empty_list));
    assert_int_equal(sz, sizeof(empty_list));
    assert_int_equal(err, 0);
    err = wire_read_sz(&sz, short_list, sizeof(short_list));
    assert_int_equal(sz, sizeof(short_list));
    assert_int_equal(err, 0);
    err = wire_read_sz(&sz, long_list, sizeof(long_list));
    assert_int_equal(sz, sizeof(long_list));
    assert_int_equal(err, 0);
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
        cmocka_unit_test(test_wire_parse),
        cmocka_unit_test(test_wire_read_size)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

