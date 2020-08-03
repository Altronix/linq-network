#include "json.h"
#include "sys.h"
#include "wire.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static char expect_bytes[4096];
static uint8_t expect_len;

static const uint8_t* recv_short_json = (uint8_t*)"";
static const uint8_t* recv_short = (uint8_t*)"\x0\xfe"
                                             "{"
                                             "\"vers\":0,"
                                             "\"type\":0,"
                                             "\"meth\":\"POST\","
                                             "\"path\":\"/api/v1/network\","
                                             "\"data\":\"{'foo':'bar'}\""
                                             "}";

// 0x01FC
static const uint8_t* recv_long =
    (uint8_t*)"\x0\xfe"
              "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
              "\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02"
              "\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03"
              "\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04"
              "\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05"
              "\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06"
              "\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07"
              "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
              "\x09\x09\x09\x09\x09\x09\x09\x09\x09\x09"
              "\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10"
              "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11"
              "\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12"
              "\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13"
              "\x14\x14\x14\x14\x14\x14\x14\x14\x14\x14"
              "\x15\x15\x15\x15\x15\x15\x15\x15\x15\x15"
              "\x16\x16\x16\x16\x16\x16\x16\x16\x16\x16"
              "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
              "\x18\x18\x18\x18\x18\x18\x18\x18\x18\x18"
              "\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19"
              "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
              "\x21\x21\x21\x21\x21\x21\x21\x21\x21\x21"
              "\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22"
              "\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23"
              "\x24\x24\x24\x24\x24\x24\x24\x24\x24\x24"
              "\x25\x25\x25\x25\x25\x25\x25\x25\x25\x25"
              "\x00\x01\x02\x03"
              "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
              "\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02"
              "\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03"
              "\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04"
              "\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05"
              "\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06"
              "\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07"
              "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
              "\x09\x09\x09\x09\x09\x09\x09\x09\x09\x09"
              "\x10\x10\x10\x10\x10\x10\x10\x10\x10\x10"
              "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11"
              "\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12"
              "\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13"
              "\x14\x14\x14\x14\x14\x14\x14\x14\x14\x14"
              "\x15\x15\x15\x15\x15\x15\x15\x15\x15\x15"
              "\x16\x16\x16\x16\x16\x16\x16\x16\x16\x16"
              "\x17\x17\x17\x17\x17\x17\x17\x17\x17\x17"
              "\x18\x18\x18\x18\x18\x18\x18\x18\x18\x18"
              "\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19"
              "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
              "\x21\x21\x21\x21\x21\x21\x21\x21\x21\x21"
              "\x22\x22\x22\x22\x22\x22\x22\x22\x22\x22"
              "\x23\x23\x23\x23\x23\x23\x23\x23\x23\x23"
              "\x24\x24\x24\x24\x24\x24\x24\x24\x24\x24"
              "\x25\x25\x25\x25\x25\x25\x25\x25\x25\x25"
              "\x00\x01\x02\x03";

void
recv_short_fn(struct wire_s* w, void* ctx, E_WIRE_EVENT e, ...)
{
    uint8_t* bytes;
    int len;
    va_list list;
    va_start(list, e);
    bytes = va_arg(list, uint8_t*);
    len = va_arg(list, int);
    assert_int_equal(expect_len, len);
    assert_int_equal(e, WIRE_EVENT_RECV);
    assert_memory_equal(expect_bytes, bytes, len);
    va_end(list);
    *((bool*)ctx) = true;
}

static void
test_usb_wire_parse_recv_short(void** context_p)
{
    wire_s wire;
    bool pass = false;

    expect_len = 254;
    memcpy(expect_bytes, recv_short + 2, 254);
    wire_init(&wire, &recv_short_fn, &pass);
    wire_parse(&wire, recv_short, 2 + 254);
    assert_true(pass);

    wire_free(&wire);
}

void
recv_short_error_fn(struct wire_s* w, void* ctx, E_WIRE_EVENT e, ...)
{
    assert_int_equal(e, WIRE_EVENT_ERROR);
    *((bool*)ctx) = true;
}

static void
test_usb_wire_parse_recv_error_version(void** context_p)
{
    wire_s wire;
    bool pass = false;

    wire_init(&wire, &recv_short_error_fn, &pass);
    wire_parse(&wire, (uint8_t*)"\x01\x02\x03", 3);
    assert_true(pass);

    wire_free(&wire);
}

void
write_short_fn(struct wire_s* w, void* ctx, E_WIRE_EVENT e, ...)
{
    va_list list;
    int len;
    uint8_t* bytes;
    va_start(list, e);
    bytes = va_arg(list, uint8_t*);
    len = va_arg(list, int);
    va_end(list);
    assert_int_equal(e, WIRE_EVENT_WANT_WRITE);
    assert_int_equal(expect_len, len);
    assert_memory_equal(expect_bytes, bytes, len);
    *((bool*)ctx) = true;
}

static void
test_usb_wire_write_short(void** context_p)
{
    bool pass = false;
    const char* expect = "{\"hello\":\"world\"}";
    wire_s wire;
    expect_len = 17 + 2;
    expect_bytes[0] = 0;
    expect_bytes[1] = 17;
    memcpy(&expect_bytes[2], expect, 17);
    wire_init(&wire, write_short_fn, &pass);
    wire_write(&wire, "{\"hello\":\"%s\"}", "world");
    assert_true(pass);
    wire_free(&wire);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_usb_wire_parse_recv_short),
        cmocka_unit_test(test_usb_wire_parse_recv_error_version),
        cmocka_unit_test(test_usb_wire_write_short)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

