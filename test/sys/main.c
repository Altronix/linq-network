#include "common/json.h"
#include "sys.h"
#include "sys/ioctl.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
test_sys_read_buffer(void** context_p)
{
    int err;
    char buffer[6];
    uint32_t len;

    spy_file_init();

    // Pass
    len = sizeof(buffer);
    spy_file_push_incoming("foobar", 6);
    err = sys_read_buffer(NULL, buffer, &len);
    assert_int_equal(len, 6);
    assert_int_equal(err, 6);
    assert_memory_equal(buffer, "foobar", 6);

    // Fail
    len = sizeof(buffer);
    spy_file_push_incoming("123456789abc", 12);
    err = sys_read_buffer(NULL, buffer, &len);
    assert_int_equal(err, 6);
    assert_int_equal(len, 12);
    assert_memory_equal(buffer, "123456", 6);

    spy_file_free();
}

static void
test_sys_read_alloc(void** context_p)
{
    int err;
    char* p = NULL;
    uint32_t len = 0;

    spy_file_init();

    // Pass
    spy_file_push_incoming("foobar", 6);
    sys_read(NULL, &p, &len);
    assert_int_equal(len, 6);
    assert_memory_equal(p, "foobar", 6);
    free(p);

    spy_file_free();
}

static void
test_sys_fprintf(void** context_p)
{
    int err;
    spy_file_packet_s* packet;

    spy_file_init();

    err = sys_fprintf(NULL, "Hello [%s]", "world");
    assert_int_equal(err, 13);
    packet = spy_file_packet_pop_outgoing();
    assert_non_null(packet);
    assert_int_equal(packet->len, 13);
    assert_memory_equal(packet->bytes, "Hello [world]", 13);
    spy_file_packet_free(&packet);

    spy_file_free();
}

static int
sys_vfprintf_test_fn(const char* fmt, ...)
{
    int ret;
    va_list list;
    va_start(list, fmt);
    ret = sys_vfprintf(NULL, fmt, list);
    va_end(list);
    return ret;
}

static void
test_sys_vfprintf(void** context_p)
{
    int err;
    spy_file_packet_s* packet;

    spy_file_init();

    err = sys_vfprintf_test_fn("Hello [%s]", "world");
    assert_int_equal(err, 13);
    packet = spy_file_packet_pop_outgoing();
    assert_non_null(packet);
    assert_int_equal(packet->len, 13);
    assert_memory_equal(packet->bytes, "Hello [world]", 13);
    spy_file_packet_free(&packet);

    spy_file_free();
}

static void
test_sys_throughput_read(void** context_p)
{
    int err, param;
    char buffer[16] = "0123456789abcdef";

    spy_file_init();

    spy_file_push_incoming(buffer, 16);
    spy_file_set_throughput(1);
    ioctl(0, 0, &param);
    assert_int_equal(param, 1);
    for (int i = 0; i < sizeof(buffer); i++) {
        uint32_t sz = sizeof(buffer) - i;
        char c;
        err = sys_read_buffer(NULL, &c, &sz);
        assert_int_equal(err, 1);
        assert_int_equal(sz, 1);
        assert_int_equal(c, buffer[i]);
    }

    spy_file_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sys_read_buffer),    //
        cmocka_unit_test(test_sys_read_alloc),     //
        cmocka_unit_test(test_sys_fprintf),        //
        cmocka_unit_test(test_sys_vfprintf),       //
        cmocka_unit_test(test_sys_throughput_read) //
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

