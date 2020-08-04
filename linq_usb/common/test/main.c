#include "json.h"
#include "rlp.h"
#include "sys.h"
#include "wire.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
test_wire_print_alloc(void** context_p)
{
    uint8_t* b = NULL;
    uint32_t l;
    int err;
    rlp* result;
    err = wire_print(&b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
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
test_wire_print_buffer(void** context_p)
{
    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e;

    rlp* result;
    e = wire_print_buffer(b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
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
test_wire_parse(void** context_p)
{
    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e = wire_print_buffer(b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
    assert_int_equal(e, 0);
    wire_parser_s parser;
    wire_parser_init(&parser);
    wire_parse(&parser, b, l);
    assert_int_equal(0, wire_parser_read_vers(&parser));
    assert_int_equal(0, wire_parser_read_type(&parser));
    assert_string_equal("POST", wire_parser_read_meth(&parser));
    assert_string_equal("/network", wire_parser_read_path(&parser));
    assert_string_equal("{\"foo\":\"bar\"}", wire_parser_read_data(&parser));
    wire_parser_free(&parser);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_wire_print_alloc), //
        cmocka_unit_test(test_wire_print_buffer),
        cmocka_unit_test(test_wire_parse)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

