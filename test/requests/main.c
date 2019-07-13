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
        "sid",
        "/ATX/network/zmtp/cloud/tls/enable",
        "{\"enable\":1}",
        test_request_complete_fn);
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
    request_s* r0 = request_create("sid0", "0", "0", test_request_complete_fn);
    request_s* r1 = request_create("sid1", "1", "1", test_request_complete_fn);
    request_s* r2 = request_create("sid2", "2", "2", test_request_complete_fn);
    request_s* no = NULL;
    request_list_s* requests = request_list_create();
    assert_int_equal(request_list_size(requests), 0);

    request_list_push(requests, &r0);
    assert_null(r0);
    assert_int_equal(request_list_size(requests), 1);

    request_list_push(requests, &r1);
    assert_null(r1);
    assert_int_equal(request_list_size(requests), 2);

    request_list_push(requests, &r2);
    assert_null(r2);
    assert_int_equal(request_list_size(requests), 3);

    r0 = request_list_pop(requests);
    assert_non_null(r0);
    assert_string_equal(request_serial_get(r0), "sid0");
    assert_int_equal(request_list_size(requests), 2);

    r1 = request_list_pop(requests);
    assert_non_null(r1);
    assert_string_equal(request_serial_get(r1), "sid1");
    assert_int_equal(request_list_size(requests), 1);

    r2 = request_list_pop(requests);
    assert_non_null(r2);
    assert_string_equal(request_serial_get(r2), "sid2");
    assert_int_equal(request_list_size(requests), 0);

    no = request_list_pop(requests);
    assert_null(no);
    assert_int_equal(request_list_size(requests), 0);

    request_destroy(&r0);
    request_destroy(&r1);
    request_destroy(&r2);
    request_list_destroy(&requests);
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
