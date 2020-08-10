#include "linq_usbh.h"
#include "log.h"

int
main(int argc, char* argv[])
{
    int rc;
    char b[128];
    uint16_t code;
    linq_usbh_s usb;
    linq_usbh_init(&usb);

    // Scan for device
    rc = linq_usbh_scan(&usb, 0x3333, 0x4444);
    if (!rc) {
        log_error("(APP) - device not found");
        linq_usbh_free(&usb);
        exit(-1);
    }

    // Send a request
    rc = linq_usbh_send_http_request_sync(&usb, "N/A", "GET", "/foo", NULL);
    if (rc < 0) {
        log_error("(APP) tx io error (%d)", rc);
        exit(-1);
    }

    // Receive response
    rc = linq_usbh_recv_http_response_sync(&usb, "N/A", &code, b, sizeof(b));
    if (rc < 0) {
        log_error("(APP) rc io error (%d)", rc);
        exit(-1);
    }

    // cleanup
    log_info("(APP) - received [%d] [%s]", code, b);
    linq_usbh_free(&usb);
}
