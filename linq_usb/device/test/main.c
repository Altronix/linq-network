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
    linq_usbd_init(&usb);
    linq_usbd_free(&usb);
}

static void
test_usb_recv_callback(linq_usbd_s* usb, void* ctx, E_USB_EVENTS e, ...)
{
    const char *meth, *path, *data;

    va_list list;
    va_start(list, e);
    meth = va_arg(list, const char*);
    path = va_arg(list, const char*);
    data = va_arg(list, const char*);
    va_end(list);
    assert_int_equal(USB_EVENTS_TYPE_HTTP, e);
    assert_string_equal("POST", meth);
    assert_string_equal("/network", path);
    assert_null(data);
    *((bool*)ctx) = true;
}

static void
test_usb_recv(void** context_p)
{
    bool pass = false;
    int err;
    linq_usbd_s usb;
    spy_file_init();

    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e = wire_print_buffer(b, &l, "POST", "/network", NULL);
    assert_int_equal(e, 0);

    linq_usbd_init(&usb);
    spy_file_push_ioctl(l);
    spy_file_push_incoming((char*)b, l);
    e = linq_usbd_poll(&usb, test_usb_recv_callback, &pass);
    assert_true(pass);

    spy_file_free();
}

static void
test_usb_recv_data_callback(linq_usbd_s* usb, void* ctx, E_USB_EVENTS e, ...)
{
    const char *meth, *path, *data;

    va_list list;
    va_start(list, e);
    meth = va_arg(list, const char*);
    path = va_arg(list, const char*);
    data = va_arg(list, const char*);
    va_end(list);
    assert_int_equal(USB_EVENTS_TYPE_HTTP, e);
    assert_string_equal("POST", meth);
    assert_string_equal("/network", path);
    assert_string_equal("{\"foo\":\"bar\"}", data);
    *((bool*)ctx) = true;
}

static void
test_usb_recv_data(void** context_p)
{
    bool pass = false;
    int err;
    linq_usbd_s usb;
    spy_file_init();

    uint8_t b[256];
    uint32_t l = sizeof(b);
    int e = wire_print_buffer(b, &l, "POST", "/network", "{\"foo\":\"bar\"}");
    assert_int_equal(e, 0);

    linq_usbd_init(&usb);
    spy_file_push_ioctl(l);
    spy_file_push_incoming((char*)b, l);
    e = linq_usbd_poll(&usb, test_usb_recv_data_callback, &pass);
    assert_true(pass);

    spy_file_free();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_usb_init),     //
        cmocka_unit_test(test_usb_recv),     //
        cmocka_unit_test(test_usb_recv_data) //
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
