#include "common/json.h"

#include <setjmp.h>

#include <cmocka.h>
static const char* sample = "{"
                            "\"sample\":\"normal\","
                            "\"word\":\"big\","
                            "\"number\":5,"
                            "\"u64\":18446744073709551615,"
                            "\"u32\":4294967295,"
                            "\"u16\":65535,"
                            "\"u8\":255,"
                            "\"song\":\"mary had a little lamb\","
                            "\"end\":true"
                            "}";

static const char* sample_nest =
    "{"
    "\"thisa\":{"
    "           \"thata\":{"
    "                     \"valA\":\"thisa-thata-vala\","
    "                     \"valB\":\"thisa-thata-valb\","
    "                     \"valC\":\"thisa-thata-valc\","
    "                     \"valD\":\"thisa-thata-vald\","
    "                     \"fooD\":{\"valD\":\"gotcha\"},"
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
    "           \"thisb\":{"
    "                     \"valA\":\"thatb-thisb-vala\","
    "                     \"valB\":\"thatb-thisb-valb\","
    "                     \"valC\":\"thatb-thisb-valc\","
    "                     \"valD\":\"thatb-thisb-vald\","
    "                     \"valE\":\"thatb-thisb-vale\""
    "           }"
    "}"
    "}";

static void
test_json_delve(void** context_p)
{
    int err;
    jsmntok_t toks[150];
    const jsmntok_t* t;
    jsmn_parser p;
    jsmn_init(&p);
    jsmn_parse(&p, sample_nest, strlen(sample_nest), toks, 150);
    t = json_delve(sample_nest, toks, ".thisa.thata.valA");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thata-vala", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thata.valB");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thata-valb", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thata.valC");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thata-valc", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thata.valD");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thata-vald", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thata.valE");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thata-vale", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thata.valF");
    assert_null(t);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valA");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thatb-vala", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valB");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thatb-valb", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valC");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thatb-valc", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valD");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thatb-vald", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valE");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thisa-thatb-vale", t->size);
    t = json_delve(sample_nest, toks, ".thisa.thatb.valF");
    assert_null(t);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valA");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisa-vala", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valB");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisa-valb", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valC");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisa-valc", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valD");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisa-vald", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valE");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisa-vale", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisa.valF");
    assert_null(t);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valA");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisb-vala", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valB");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisb-valb", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valC");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisb-valc", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valD");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisb-vald", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valE");
    assert_non_null(t);
    assert_memory_equal(&sample_nest[t->start], "thatb-thisb-vale", t->size);
    t = json_delve(sample_nest, toks, ".thatb.thisb.valF");
    assert_null(t);
}

static void
test_json_to_number(void** context_p)
{
    int err;
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
    jsmntok_t toks[32];
    const jsmntok_t* t;
    jsmn_parser p;
    jsmn_init(&p);
    jsmn_parse(&p, sample, strlen(sample), toks, 32);
    // u32
    t = json_get_member(sample, toks, "u32");
    assert_non_null(t);
    err = json_to_u32(sample, t, &u32);
    assert_int_equal(err, 0);
    assert_int_equal(u32, 4294967295);

    // u16
    t = json_get_member(sample, toks, "u16");
    assert_non_null(t);
    err = json_to_u16(sample, t, &u16);
    assert_int_equal(err, 0);
    assert_int_equal(u16, 65535);

    // u8
    t = json_get_member(sample, toks, "u8");
    assert_non_null(t);
    err = json_to_u8(sample, t, &u8);
    assert_int_equal(err, 0);
    assert_int_equal(u8, 255);
}

static void
test_json_get_member(void** context_p)
{
    jsmntok_t toks[32];
    jsmn_parser p;
    jsmn_init(&p);
    uint32_t x;
    int err, n = jsmn_parse(&p, sample, strlen(sample), toks, 32);
    const jsmntok_t* t = json_get_member(sample, toks, "number");
    assert_non_null(t);
    err = json_to_u32(sample, t, &x);
    assert_int_equal(err, 0);
    assert_int_equal(x, 5);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_json_delve),
        cmocka_unit_test(test_json_to_number),
        cmocka_unit_test(test_json_get_member),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

