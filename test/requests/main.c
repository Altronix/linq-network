#include "device.h"
#include "request.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_request_complete_fn(E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)e);
    ((void)json);
    ((void)d);
}

static void
test_request_create(void** context_p)
{
    ((void)context_p);
    request_s* request = request_create(
        "sid", "/ATX/exe/save", "{\"save\":1}", test_request_complete_fn);
    request_list_s* requests = request_list_create();
    assert_non_null(requests);
    assert_non_null(request);

    request_list_push(requests, &request);

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
                                        cmocka_unit_test(test_request_insert) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
