#include "json.h"
#include "linq_usbh.h"
#include "sys.h"

#include <setjmp.h>

#include "mock_libusb.h"

#include <cmocka.h>

static void
test_scan(void** context_p)
{
    spy_libusb_init();
    struct libusb_device_descriptor dev0 = { .idProduct = 1111,
                                             .idVendor = 2222 },
                                    dev1 = { .idProduct = 3333,
                                             .idVendor = 4444 },
                                    dev2 = { .idProduct = 3333,
                                             .idVendor = 4444 };
    libusb_context* context;
    libusb_init(&context);

    spy_libusb_push_device(&dev0);

    libusb_exit(context);
    spy_libusb_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_scan)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
