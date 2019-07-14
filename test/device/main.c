#include "altronix/linq.h"
#include "device.h"
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static E_LINQ_ERROR expect_error = LINQ_ERROR_OK;
static const char* empty = "";
static const char* expect_what = "";
static const char* expect_serial = "";

static void
test_reset()
{
    expect_error = LINQ_ERROR_OK;
    expect_what = empty;
    expect_serial = empty;
    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
}

static void
test_device_create(void** context_p)
{
    ((void)context_p);
    zsock_t* sock = NULL;
    device_s* d = device_create(&sock, (uint8_t*)"rid", 3, "sid", "pid");
    assert_non_null(d);

    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_get(d, "ATX", NULL);
    device_send_post(d, "ATX", "{\"test\":1}", NULL);
    assert_int_equal(device_request_pending_count(d), 5);

    device_destroy(&d);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_device_create),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
