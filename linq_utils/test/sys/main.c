#include "json.h"

#include <setjmp.h>

#include <cmocka.h>

static void
test_sys_read_buffer(void** context_p)
{}

static void
test_sys_read_alloc(void** context_p)
{}

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

