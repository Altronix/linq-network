#include "log.h"
#include "usbh.h"

int
main(int argc, char* argv[])
{
    int rc;
    char b[128];
    uint16_t code;
    usbh_s usb;
    // TODO use netw_() wrapper
    device_map_s* devices = device_map_create();
    usbh_init(&usb, &devices);

    // Scan for device
    rc = usbh_scan(&usb, 0x3333, 0x4444);
    if (!rc) {
        log_error("(APP) - device not found");
        usbh_free(&usb);
        exit(-1);
    }

    // Send a request
    rc = usbh_send_http_request_sync(&usb, "N/A", "GET", "/foo", NULL);
    if (rc < 0) {
        log_error("(APP) tx io error (%d)", rc);
        exit(-1);
    }

    // Receive response
    rc = usbh_recv_http_response_sync(&usb, "N/A", &code, b, sizeof(b));
    if (rc < 0) {
        log_error("(APP) rc io error (%d)", rc);
        exit(-1);
    }

    // cleanup
    log_info("(APP) - received [%d] [%s]", code, b);
    device_map_destroy(&devices); // Note devices must be free'd before usb
    usbh_free(&usb);
}
