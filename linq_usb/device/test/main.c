#include "json.h"
#include "linq_usbd.h"
#include "sys.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

static void
usbd_event(
    linq_usbd_s* usb,
    E_USB_EVENTS e,
    const char* b,
    uint32_t l,
    void* ctx)
{
    assert_int_equal(l, 3);
    assert_memory_equal(b, "foo", 3);
    *((bool*)ctx) = true;
}

struct linq_usbd_callbacks_s callbacks = { .event = &usbd_event };

static void
test_usb_init(void** context_p)
{
    linq_usbd_s usb;
    linq_usbd_init(&usb, NULL, NULL);
    linq_usbd_free(&usb);
}

static void
test_usb_recv(void** context_p)
{
    bool pass = false;
    int err;
    spy_file_init();

    /*
     // TODO test needs to handle wire and application layer data
    linq_usbd_s usb;
    linq_usbd_init(&usb, &callbacks, &pass);
    err = linq_usbd_poll(&usb);
    assert_int_equal(err, 0);
    assert_false(pass);
    spy_file_push_ioctl(3);
    spy_file_push_incoming("foo", 3);
    err = linq_usbd_poll(&usb);
    assert_int_equal(err, 3);
    assert_true(pass);
    */

    spy_file_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_usb_init), //
        cmocka_unit_test(test_usb_recv)  //
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
