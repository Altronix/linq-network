#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <setjmp.h>

#include <cmocka.h>

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
