#include "io_m5.h"
#include "json.h"
#include "linq_usbh.h"
#include "sys.h"

#include <setjmp.h>

#include "mock_libusb.h"

#include <cmocka.h>

static void
test_io_recv(void** context_p)
{
    struct libusb_device_descriptor desc = { .idVendor = 1111,
                                             .idProduct = 2222 };
    spy_libusb_init();
    io_s* io = io_m5_init(NULL, desc);

    io_m5_free(&io);
    spy_libusb_free();
}

static void
test_scan(void** context_p)
{
    spy_libusb_init();
    char buffer[512];
    int err = 0;
    struct libusb_device_descriptor dev0 = { .idVendor = 1111,
                                             .idProduct = 2222 },
                                    dev1 = { .idVendor = 3333,
                                             .idProduct = 4444 },
                                    dev2 = { .idVendor = 5555,
                                             .idProduct = 6666 };
    linq_usbh_s usb;
    linq_usbh_init(&usb);

    spy_libusb_push_device(&dev0, "string-0");
    spy_libusb_push_device(&dev1, "string-1");
    spy_libusb_push_device(&dev2, "string-2");
    err = linq_usbh_scan(&usb, 0000, 0000);
    assert_int_equal(err, 0);
    assert_int_equal(linq_usbh_device_count(&usb), 0);
    err = linq_usbh_scan(&usb, 1111, 2222);
    assert_int_equal(err, 1);
    assert_int_equal(linq_usbh_device_count(&usb), 1);
    err = linq_usbh_scan(&usb, 3333, 4444);
    assert_int_equal(err, 1);
    assert_int_equal(linq_usbh_device_count(&usb), 2);
    err = linq_usbh_scan(&usb, 5555, 6666);
    assert_int_equal(err, 1);
    assert_int_equal(linq_usbh_device_count(&usb), 3);

    err = linq_usbh_print_devices(&usb, buffer, 512);
    json_parser p;
    jsontok t[128];
    json_init(&p);
    err = json_parse(&p, buffer, err, t, sizeof(buffer));
    assert_true(err > 0);
    json_value idvend, idprod, mfg, prod, serial;
    idvend = json_delve_value(buffer, t, ".string-0.idVendor");
    idprod = json_delve_value(buffer, t, ".string-0.idProduct");
    assert_true(idvend.p && idprod.p);
    assert_memory_equal(idvend.p, "1111", idvend.len);
    assert_memory_equal(idprod.p, "2222", idprod.len);

    linq_usbh_free(&usb);
    spy_libusb_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_scan),
                                        cmocka_unit_test(test_io_recv)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
