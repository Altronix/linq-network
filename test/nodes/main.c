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

    // TODO - add some nodes

    nodes_destroy(&dm);
    assert_null(dm);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_nodes_create) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
