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
    assert_non_null(dm);

    node_s* n = node_recv(NULL);
    nodes_insert(dm, "test", &n);

    // TODO - add some nodes

    nodes_destroy(&dm);
    assert_null(dm);
}

static void
test_nodes_insert(void** context_p)
{
    ((void)context_p);

    nodes_s* n = nodes_create();
    node_s *n0 = node_recv(NULL), *n1 = node_recv(NULL), *n2 = node_recv(NULL),
           **r0, **r1, **r2, **node;

    assert_int_equal(nodes_size(n), 0);

    r0 = nodes_insert(n, "n0", &n0);
    assert_null(n0);
    assert_int_equal(nodes_size(n), 1);

    r1 = nodes_insert(n, "n1", &n1);
    assert_null(n1);
    assert_int_equal(nodes_size(n), 2);

    r2 = nodes_insert(n, "n2", &n2);
    assert_null(n2);
    assert_int_equal(nodes_size(n), 3);

    node = nodes_get(n, "does not exist");
    assert_null(node);

    node = nodes_get(n, "n0");
    assert_non_null(node);
    assert_int_equal(*r0, *node);

    node = nodes_get(n, "n1");
    assert_non_null(node);
    assert_int_equal(*r1, *node);

    node = nodes_get(n, "n2");
    assert_non_null(node);
    assert_int_equal(*r2, *node);

    nodes_remove(n, "does not exist");
    assert_int_equal(nodes_size(n), 3);

    nodes_remove(n, "n0");
    assert_int_equal(nodes_size(n), 2);
    nodes_remove(n, "n1");
    assert_int_equal(nodes_size(n), 1);
    nodes_remove(n, "n2");
    assert_int_equal(nodes_size(n), 0);

    nodes_destroy(&n);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_nodes_create),
                                        cmocka_unit_test(test_nodes_insert) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
