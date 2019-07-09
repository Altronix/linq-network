#include "altronix/linq.h"
#include "mock_zmsg.h"

#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_incoming(void** context_p)
{
    ((void)context_p);
}

static void
test_linq_create(void** context_p)
{
    ((void)context_p);
    linq* l = linq_create(NULL, NULL);
    assert_non_null(l);
    linq_destroy(&l);
    assert_null(l);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_mock_incoming),
                                        cmocka_unit_test(test_linq_create) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
