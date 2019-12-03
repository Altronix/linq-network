// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "containers.h"
#include "device.h"
#include "helpers.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

MAP_INIT(node, device_s, device_destroy);

static void
test_nodes_create(void** context_p)
{
    ((void)context_p);
    node_map_s* dm = node_map_create();
    device_s* node = device_create(NULL, (uint8_t*)"rid", 4, "sid", "pid");
    assert_non_null(dm);
    node_map_add(dm, device_serial(node), &node);
    node_map_destroy(&dm);
    assert_null(dm);
}

static void
test_nodes_add(void** context_p)
{
    ((void)context_p);

    node_map_s* m = node_map_create();
    device_s** d;
    router_s rid0 = { "rid0", 4 };
    router_s rid1 = { "rid1", 4 };
    router_s rid2 = { "rid2", 4 };
    device_s *n0 = device_create(NULL, (uint8_t*)"rid0", 4, "sid0", "pid0"),
             *n1 = device_create(NULL, (uint8_t*)"rid1", 4, "sid1", "pid1"),
             *n2 = device_create(NULL, (uint8_t*)"rid2", 4, "sid2", "pid2");

    assert_int_equal(node_map_size(m), 0);
    node_map_add(m, device_serial(n0), &n0);
    assert_int_equal(node_map_size(m), 1);
    node_map_add(m, device_serial(n1), &n1);
    assert_int_equal(node_map_size(m), 2);
    node_map_add(m, device_serial(n2), &n2);
    assert_int_equal(node_map_size(m), 3);

    d = node_map_get(m, "does not exist");
    assert_null(d);

    d = node_map_get(m, "sid0");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid0, sizeof(rid0));
    assert_string_equal(device_serial(*d), "sid0");
    assert_string_equal(device_type(*d), "pid0");

    d = node_map_get(m, "sid1");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid1, sizeof(rid1));
    assert_string_equal(device_serial(*d), "sid1");
    assert_string_equal(device_type(*d), "pid1");

    d = node_map_get(m, "sid2");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid2, sizeof(rid2));
    assert_string_equal(device_serial(*d), "sid2");
    assert_string_equal(device_type(*d), "pid2");

    node_map_remove(m, "does not exist");
    assert_int_equal(node_map_size(m), 3);
    node_map_remove(m, "sid0");
    assert_int_equal(node_map_size(m), 2);
    node_map_remove(m, "sid1");
    assert_int_equal(node_map_size(m), 1);
    node_map_remove(m, "sid2");
    assert_int_equal(node_map_size(m), 0);

    node_map_destroy(&m);
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
