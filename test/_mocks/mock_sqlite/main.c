#include "altronix/linq_network.h"

#include "helpers.h"
#include "mock_sqlite.h"
#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_sqlite_outgoing(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mock_sqlite_outgoing),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
