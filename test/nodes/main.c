#include "node.h"
#include "nodes.h"
#include "helpers.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_nodes_create(void** context_p)
{
    ((void)context_p);
    nodes_s* dm = nodes_create();
    assert_non_null(dm);
    nodes_insert(dm, NULL, (uint8_t*)"rid", 3, "test", "test");
    nodes_destroy(&dm);
    assert_null(dm);
}

static void
test_nodes_insert(void** context_p)
{
    ((void)context_p);

    nodes_s* m = nodes_create();
    node_s** d;
    router_s rid0 = { "router0", 7 };
    router_s rid1 = { "router1", 7 };
    router_s rid2 = { "router2", 7 };
    zsock_t* sock = NULL;

    assert_int_equal(nodes_size(m), 0);
    nodes_insert(m, &sock, (uint8_t*)"router0", 7, "serial0", "product0");
    assert_int_equal(nodes_size(m), 1);
    nodes_insert(m, &sock, (uint8_t*)"router1", 7, "serial1", "product1");
    assert_int_equal(nodes_size(m), 2);
    nodes_insert(m, &sock, (uint8_t*)"router2", 7, "serial2", "product2");
    assert_int_equal(nodes_size(m), 3);

    d = nodes_get(m, "does not exist");
    assert_null(d);

    d = nodes_get(m, "serial0");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid0, sizeof(rid0));
    assert_string_equal(node_serial(*d), "serial0");
    assert_string_equal(node_product(*d), "product0");

    d = nodes_get(m, "serial1");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid1, sizeof(rid1));
    assert_string_equal(node_serial(*d), "serial1");
    assert_string_equal(node_product(*d), "product1");

    d = nodes_get(m, "serial2");
    assert_non_null(d);
    assert_memory_equal(node_router(*d), &rid2, sizeof(rid2));
    assert_string_equal(node_serial(*d), "serial2");
    assert_string_equal(node_product(*d), "product2");

    nodes_remove(m, "does not exist");
    assert_int_equal(nodes_size(m), 3);
    nodes_remove(m, "serial0");
    assert_int_equal(nodes_size(m), 2);
    nodes_remove(m, "serial1");
    assert_int_equal(nodes_size(m), 1);
    nodes_remove(m, "serial2");
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
                                        cmocka_unit_test(test_nodes_insert)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
