#include "containers.h"
#include "device.h"
#include "device_request.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

LIST_INIT(requests, device_request_s, device_request_destroy);

static void
on_request(void* context, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)context);
    ((void)e);
    ((void)json);
    ((void)d);
}

static void
test_request_create(void** context_p)
{
    ((void)context_p);
    device_request_s* request = device_request_create(
        REQUEST_METHOD_POST,
        "sid",
        "/ATX/network/zmtp/cloud/tls/enable",
        "{\"enable\":1}",
        on_request,
        NULL);
    list_requests_s* requests = list_requests_create();
    assert_non_null(requests);
    assert_non_null(request);

    list_requests_push(requests, &request);

    list_requests_destroy(&requests);
    assert_null(requests);
}

static void
test_request_insert(void** context_p)
{
    ((void)context_p);
    device_request_s* r0 = device_request_create(
        REQUEST_METHOD_POST, "sid0", "0", "0", on_request, NULL);
    device_request_s* r1 = device_request_create(
        REQUEST_METHOD_POST, "sid1", "1", "1", on_request, NULL);
    device_request_s* r2 = device_request_create(
        REQUEST_METHOD_POST, "sid2", "2", "2", on_request, NULL);
    device_request_s* no = NULL;
    list_requests_s* requests = list_requests_create();
    assert_int_equal(list_requests_size(requests), 0);

    list_requests_push(requests, &r0);
    assert_null(r0);
    assert_int_equal(list_requests_size(requests), 1);

    list_requests_push(requests, &r1);
    assert_null(r1);
    assert_int_equal(list_requests_size(requests), 2);

    list_requests_push(requests, &r2);
    assert_null(r2);
    assert_int_equal(list_requests_size(requests), 3);

    r0 = list_requests_pop(requests);
    assert_non_null(r0);
    assert_string_equal(device_request_serial_get(r0), "sid0");
    assert_int_equal(list_requests_size(requests), 2);

    r1 = list_requests_pop(requests);
    assert_non_null(r1);
    assert_string_equal(device_request_serial_get(r1), "sid1");
    assert_int_equal(list_requests_size(requests), 1);

    r2 = list_requests_pop(requests);
    assert_non_null(r2);
    assert_string_equal(device_request_serial_get(r2), "sid2");
    assert_int_equal(list_requests_size(requests), 0);

    no = list_requests_pop(requests);
    assert_null(no);
    assert_int_equal(list_requests_size(requests), 0);

    device_request_destroy(&r0);
    device_request_destroy(&r1);
    device_request_destroy(&r2);
    list_requests_destroy(&requests);
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
