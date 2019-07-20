#include "helpers.h"
#include "node.h"
#include "nodes.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_nodes_create(void** context_p)
{
    ((void)context_p);
    nodes_s* dm = nodes_create();
    node_s* node = node_create(NULL, (uint8_t*)"rid", 4, "sid", "pid");
    assert_non_null(dm);
    // nodes_add(dm, NULL, (uint8_t*)"rid", 3, "test", "test");
    nodes_add(dm, &node);
    nodes_destroy(&dm);
    assert_null(dm);
}

static void
test_nodes_add(void** context_p)
{
    ((void)context_p);

    nodes_s* m = nodes_create();
    node_s** d;
    zsock_t* sock = NULL;
    router_s rid0 = { "rid0", 4 };
    router_s rid1 = { "rid1", 4 };
    router_s rid2 = { "rid2", 4 };
    node_s *n0 = node_create(&sock, (uint8_t*)"rid0", 4, "sid0", "pid0"),
           *n1 = node_create(&sock, (uint8_t*)"rid1", 4, "sid1", "pid1"),
           *n2 = node_create(&sock, (uint8_t*)"rid2", 4, "sid2", "pid2");

    assert_int_equal(nodes_size(m), 0);
    nodes_add(m, &n0);
    assert_int_equal(nodes_size(m), 1);
    nodes_add(m, &n1);
    assert_int_equal(nodes_size(m), 2);
    nodes_add(m, &n2);
    assert_int_equal(nodes_size(m), 3);

    d = nodes_get(m, "does not exist");
    assert_null(d);

    d = nodes_get(m, "sid0");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid0, sizeof(rid0));
    assert_string_equal(node_serial(*d), "sid0");
    assert_string_equal(node_type(*d), "pid0");

    d = nodes_get(m, "sid1");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid1, sizeof(rid1));
    assert_string_equal(node_serial(*d), "sid1");
    assert_string_equal(node_type(*d), "pid1");

    d = nodes_get(m, "sid2");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid2, sizeof(rid2));
    assert_string_equal(node_serial(*d), "sid2");
    assert_string_equal(node_type(*d), "pid2");

    nodes_remove(m, "does not exist");
    assert_int_equal(nodes_size(m), 3);
    nodes_remove(m, "sid0");
    assert_int_equal(nodes_size(m), 2);
    nodes_remove(m, "sid1");
    assert_int_equal(nodes_size(m), 1);
    nodes_remove(m, "sid2");
    assert_int_equal(nodes_size(m), 0);

    nodes_destroy(&m);
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
