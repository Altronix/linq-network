#include "device.h"
#include "devices.h"
#include "helpers.h"
#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>

static void
test_devices_create(void** context_p)
{
    ((void)context_p);
    devices_s* dm = devices_create();
    assert_non_null(dm);
    devices_insert(dm, NULL, (uint8_t*)"rid", 3, "test", "test");
    devices_destroy(&dm);
    assert_null(dm);
}

static void
test_devices_insert(void** context_p)
{
    ((void)context_p);

    devices_s* m = devices_create();
    device_s** d;
    router_s rid0 = { "router0", 7 };
    router_s rid1 = { "router1", 7 };
    router_s rid2 = { "router2", 7 };
    zsock_t* sock = NULL;

    assert_int_equal(devices_size(m), 0);
    devices_insert(m, &sock, (uint8_t*)"router0", 7, "serial0", "product0");
    assert_int_equal(devices_size(m), 1);
    devices_insert(m, &sock, (uint8_t*)"router1", 7, "serial1", "product1");
    assert_int_equal(devices_size(m), 2);
    devices_insert(m, &sock, (uint8_t*)"router2", 7, "serial2", "product2");
    assert_int_equal(devices_size(m), 3);

    d = devices_get(m, "does not exist");
    assert_null(d);

    d = devices_get(m, "serial0");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid0, sizeof(rid0));
    assert_string_equal(device_serial(*d), "serial0");
    assert_string_equal(device_product(*d), "product0");

    d = devices_get(m, "serial1");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid1, sizeof(rid1));
    assert_string_equal(device_serial(*d), "serial1");
    assert_string_equal(device_product(*d), "product1");

    d = devices_get(m, "serial2");
    assert_non_null(d);
    assert_memory_equal(device_router(*d), &rid2, sizeof(rid2));
    assert_string_equal(device_serial(*d), "serial2");
    assert_string_equal(device_product(*d), "product2");

    devices_remove(m, "does not exist");
    assert_int_equal(devices_size(m), 3);
    devices_remove(m, "serial0");
    assert_int_equal(devices_size(m), 2);
    devices_remove(m, "serial1");
    assert_int_equal(devices_size(m), 1);
    devices_remove(m, "serial2");
    assert_int_equal(devices_size(m), 0);

    devices_destroy(&m);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_devices_create),
                                        cmocka_unit_test(test_devices_insert)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
