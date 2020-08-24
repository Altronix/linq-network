#include "device.h"
#include "io_m5.h"
#include "json.h"
#include "sys.h"
#include "usbh.h"

#include <setjmp.h>

#include "mock_libusb.h"

#include <cmocka.h>

static void
test_io_recv(void** context_p)
{
    usbh_s usb;
    struct libusb_device_descriptor dev0 = { .idVendor = 1111,
                                             .idProduct = 2222 };
    spy_libusb_init();

    spy_libusb_free();
}

static void
test_scan(void** context_p)
{
    spy_libusb_init();
    int err = 0;
    struct libusb_device_descriptor dev0 = { .idVendor = 1111,
                                             .idProduct = 2222 },
                                    dev1 = { .idVendor = 3333,
                                             .idProduct = 4444 },
                                    dev2 = { .idVendor = 5555,
                                             .idProduct = 6666 };
    device_map_s* devices = device_map_create();
    usbh_s usb;
    usbh_init(&usb, &devices);

    spy_libusb_push_device(&dev0, "string-0");
    spy_libusb_push_device(&dev1, "string-1");
    spy_libusb_push_device(&dev2, "string-2");
    err = usbh_scan(&usb, 0000, 0000);
    assert_int_equal(err, 0);
    assert_int_equal(usbh_device_count(&usb), 0);
    err = usbh_scan(&usb, 1111, 2222);
    assert_int_equal(err, 1);
    assert_int_equal(usbh_device_count(&usb), 1);
    err = usbh_scan(&usb, 3333, 4444);
    assert_int_equal(err, 1);
    assert_int_equal(usbh_device_count(&usb), 2);
    err = usbh_scan(&usb, 5555, 6666);
    assert_int_equal(err, 1);
    assert_int_equal(usbh_device_count(&usb), 3);

    usbh_free(&usb);
    device_map_destroy(&devices);
    spy_libusb_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_scan),
        cmocka_unit_test(test_io_recv),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
