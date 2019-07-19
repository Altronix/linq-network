#include "altronix/linq.h"
#include "device.h"
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "node.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_node_create(void** context_p)
{
    ((void)context_p);
    node_s* node = node_connect("ipc://test");
    assert_non_null(node);
    node_destroy(&node);
    assert_null(node);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { //
                                        cmocka_unit_test(test_node_create)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
