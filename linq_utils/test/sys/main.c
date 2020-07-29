#include "json.h"
#include "sys.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
test_sys_read_buffer(void** context_p)
{
    int err;
    char buffer[6], *p = buffer;
    uint32_t len = sizeof(buffer);

    spy_file_init();

    // TODO need ioctl mock

    spy_file_free();
}

static void
test_sys_read_alloc(void** context_p)
{
    spy_file_init();

    spy_file_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_sys_read_buffer),
                                        cmocka_unit_test(test_sys_read_alloc) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

