#include "device_map.h"
#include "helpers.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_device_map_create(void** context_p)
{
    ((void)context_p);
    device_map* dm = device_map_create();
    assert_non_null(dm);
    device_map_destroy(&dm);
    assert_null(dm);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_device_map_create)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
