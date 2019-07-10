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

static void
test_device_map_insert(void** context_p)
{
    ((void)context_p);

    device_map* m = device_map_create();
    zframe_t* rid = zframe_new("router", 3);
    zframe_t* sid = zframe_new("serial", 3);
    zframe_t* pid = zframe_new("product", 3);
    zsock_t* sock = NULL;

    device_map_insert(m, &sock, rid, sid, pid);
    zframe_destroy(&rid);
    zframe_destroy(&sid);
    zframe_destroy(&pid);

    device_map_destroy(&m);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_device_map_create),
        cmocka_unit_test(test_device_map_insert)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
