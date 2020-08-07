#include "mock_file.h"
#include "sys.h"
#include <sys/ioctl.h>

#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_push_incoming(void** context_p)
{
    int param;
    char buffer[6];

    spy_file_init();
    spy_file_push_incoming("foobar", sizeof(buffer));
    ioctl(0, 0, &param);
    assert_int_equal(param, 6);
    fread(buffer, 1, sizeof(buffer), NULL);
    ioctl(0, 0, &param);
    assert_int_equal(param, 0);
    assert_memory_equal(buffer, "foobar", sizeof(buffer));
    spy_file_free();
}

static void
test_mock_push_outgoing(void** context_p)
{
    spy_file_init();

    spy_file_packet_s* packet = spy_file_packet_pop_outgoing();
    assert_null(packet);

    fwrite("foobar", 1, 6, NULL);
    packet = spy_file_packet_pop_outgoing();
    assert_non_null(packet);
    assert_int_equal(packet->len, 6);
    assert_memory_equal(packet->bytes, "foobar", packet->len);
    spy_file_packet_free(&packet);

    spy_file_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mock_push_incoming),
        cmocka_unit_test(test_mock_push_outgoing),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
