#include "sys.h"

#include <setjmp.h>

#include "cmocka.h"

static void
test_parse_config(void** context_p)
{}

static void
test_print_config(void** context_p)
{}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parse_config),
        cmocka_unit_test(test_print_config),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

