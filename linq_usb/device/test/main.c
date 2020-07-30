#include "json.h"
#include "linq_usbd.h"
#include "sys.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
test_usb_init(void** context_p)
{
    linq_usbd_s usb;
    linq_usbd_init(&usb, NULL);
    linq_usbd_free(&usb);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_usb_init) //
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

