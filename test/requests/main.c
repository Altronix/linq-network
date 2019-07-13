#include "helpers.h"
#include "request.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_request_create(void** context_p)
{
    ((void)context_p);
    request_list_s* requests = request_list_create();
    assert_non_null(requests);

    request_list_destroy(&requests);
    assert_null(requests);
}

static void
test_request_insert(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_request_create),
                                        cmocka_unit_test(test_request_insert)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
