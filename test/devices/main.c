#include "containers.h"
#include "device.h"
#include "helpers.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

MAP_INIT(nodes, device_s, device_destroy);

static void
test_nodes_create(void** context_p)
{
    ((void)context_p);
    map_nodes_s* dm = map_nodes_create();
    device_s* node = device_create(NULL, (uint8_t*)"rid", 4, "sid", "pid");
    assert_non_null(dm);
    map_nodes_add(dm, device_serial(node), &node);
    map_nodes_destroy(&dm);
    assert_null(dm);
}

static void
test_nodes_add(void** context_p)
{
    ((void)context_p);

    map_nodes_s* m = map_nodes_create();
    device_s** d;
    zsock_t* sock = NULL;
    router_s rid0 = { "rid0", 4 };
    router_s rid1 = { "rid1", 4 };
    router_s rid2 = { "rid2", 4 };
    device_s *n0 = device_create(&sock, (uint8_t*)"rid0", 4, "sid0", "pid0"),
             *n1 = device_create(&sock, (uint8_t*)"rid1", 4, "sid1", "pid1"),
             *n2 = device_create(&sock, (uint8_t*)"rid2", 4, "sid2", "pid2");

    assert_int_equal(map_nodes_size(m), 0);
    map_nodes_add(m, device_serial(n0), &n0);
    assert_int_equal(map_nodes_size(m), 1);
    map_nodes_add(m, device_serial(n1), &n1);
    assert_int_equal(map_nodes_size(m), 2);
    map_nodes_add(m, device_serial(n2), &n2);
    assert_int_equal(map_nodes_size(m), 3);

    d = map_nodes_get(m, "does not exist");
    assert_null(d);

    d = map_nodes_get(m, "sid0");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid0, sizeof(rid0));
    assert_string_equal(device_serial(*d), "sid0");
    assert_string_equal(device_type(*d), "pid0");

    d = map_nodes_get(m, "sid1");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid1, sizeof(rid1));
    assert_string_equal(device_serial(*d), "sid1");
    assert_string_equal(device_type(*d), "pid1");

    d = map_nodes_get(m, "sid2");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid2, sizeof(rid2));
    assert_string_equal(device_serial(*d), "sid2");
    assert_string_equal(device_type(*d), "pid2");

    map_nodes_remove(m, "does not exist");
    assert_int_equal(map_nodes_size(m), 3);
    map_nodes_remove(m, "sid0");
    assert_int_equal(map_nodes_size(m), 2);
    map_nodes_remove(m, "sid1");
    assert_int_equal(map_nodes_size(m), 1);
    map_nodes_remove(m, "sid2");
    assert_int_equal(map_nodes_size(m), 0);

    map_nodes_destroy(&m);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_nodes_create),
                                        cmocka_unit_test(test_nodes_add)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
