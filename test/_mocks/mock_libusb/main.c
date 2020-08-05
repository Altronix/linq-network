#include "mock_libusb.h"
#include "sys.h"
#include <sys/ioctl.h>

#include <setjmp.h>

#include <cmocka.h>

static void
test_libusb_get_device_descriptor(void** context_p)
{
    spy_libusb_init();
    struct libusb_device_descriptor desc, desc0, desc1, desc2;
    desc0.idProduct = 1;
    desc1.idProduct = 2;
    desc2.idProduct = 3;
    spy_libusb_push_device(&desc0);
    spy_libusb_push_device(&desc1);
    spy_libusb_push_device(&desc2);

    libusb_device** devs;
    int err, count = libusb_get_device_list(NULL, &devs);
    assert_int_equal(count, 3);

    err = libusb_get_device_descriptor(devs[0], &desc);
    assert_int_equal(err, 0);
    assert_int_equal(desc.idProduct, 1);

    err = libusb_get_device_descriptor(devs[1], &desc);
    assert_int_equal(err, 0);
    assert_int_equal(desc.idProduct, 2);

    err = libusb_get_device_descriptor(devs[2], &desc);
    assert_int_equal(err, 0);
    assert_int_equal(desc.idProduct, 3);

    libusb_free_device_list(devs, 1);
    spy_libusb_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_libusb_get_device_descriptor),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
