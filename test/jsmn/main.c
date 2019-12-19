#include "altronix/linq_netw.h"
#include "jsmn_helpers.h"

#include <setjmp.h>

#include <cmocka.h>

static const char* data_obj = //
    "{"
    "\"hello\":\"value-hello\","
    "\"this\" :\"value-this\","
    "\"is\"   :\"value-is\","
    "\"a\"    :\"value-a\","
    "\"test\" :\"value-test\""
    "}";

static const char* data_path = //
    "{"
    "\"thisa\":{"
    "           \"thata\":{"
    "                     \"valA\":\"thisa-thata-vala\","
    "                     \"valB\":\"thisa-thata-valb\","
    "                     \"valC\":\"thisa-thata-valc\","
    "                     \"valD\":\"thisa-thata-vald\","
    "                     \"valE\":\"thisa-thata-vale\""
    "           },"
    "           \"thatb\":{"
    "                     \"valA\":\"thisa-thatb-vala\","
    "                     \"valB\":\"thisa-thatb-valb\","
    "                     \"valC\":\"thisa-thatb-valc\","
    "                     \"valD\":\"thisa-thatb-vald\","
    "                     \"valE\":\"thisa-thatb-vale\""
    "           }"
    "},"
    "\"thatb\":{"
    "           \"thisa\":{"
    "                     \"valA\":\"thatb-thisa-vala\","
    "                     \"valB\":\"thatb-thisa-valb\","
    "                     \"valC\":\"thatb-thisa-valc\","
    "                     \"valD\":\"thatb-thisa-vald\","
    "                     \"valE\":\"thatb-thisa-vale\""
    "           },"
    "           \"thatb\":{"
    "                     \"valA\":\"thatb-thisb-vala\","
    "                     \"valB\":\"thatb-thisb-valb\","
    "                     \"valC\":\"thatb-thisb-valc\","
    "                     \"valD\":\"thatb-thisb-vald\","
    "                     \"valE\":\"thatb-thisb-vale\""
    "           }"
    "}"
    "}";

static void
test_parse_obj(void** context_p)
{
    ((void)context_p);

    int count = 0;
    linq_str values[5];
    jsmntok_t t[30];

    // clang-format off
    count = jsmn_parse_tokens(
        t,
        30,
        data_obj,
        strlen(data_obj),
        5,
        "hello", &values[0],
        "this",  &values[1],
        "is",    &values[2],
        "a",     &values[3],
        "test",  &values[4]);
    // clang-format on

    assert_int_equal(count, 5);
    assert_int_equal(values[0].len, 11);
    assert_memory_equal(values[0].p, "value-hello", 11);
    assert_int_equal(values[1].len, 10);
    assert_memory_equal(values[1].p, "value-this", 10);
    assert_int_equal(values[2].len, 8);
    assert_memory_equal(values[2].p, "value-is", 8);
    assert_int_equal(values[3].len, 7);
    assert_memory_equal(values[3].p, "value-a", 7);
    assert_int_equal(values[4].len, 10);
    assert_memory_equal(values[4].p, "value-test", 10);
}

static void
test_parse_path(void** context_p)
{
    ((void)context_p);

    int count = 0;
    linq_str values[5];
    jsmntok_t t[30];

    // clang-format off
    /*
    count = jsmn_parse_tokens_path(
        "/thatb/thisa",
        t,
        150,
        data_path,
        strlen(data_path),
        5,
        "hello", &values[0],
        "this",  &values[1],
        "is",    &values[2],
        "a",     &values[3],
        "test",  &values[4]);
        */
    // clang-format on
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_parse_obj),
                                        cmocka_unit_test(test_parse_path) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
