#include "altronix/linq_netw.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

static void
test_route_devices(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_route_devices) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
