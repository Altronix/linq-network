#include "linq_usbh.h"
#include "log.h"

int
main(int argc, char* argv[])
{
    char b[128];
    uint16_t code;
    linq_usbh_s usb;
    linq_usbh_init(&usb);
    linq_usbh_scan(&usb, 0x3333, 0x4444);
    linq_usbh_send_http_request_sync(&usb, "N/A", "GET", "/api/v1/foo", NULL);
    linq_usbh_recv_http_response_sync(&usb, "N/A", &code, b, sizeof(b));
    log_info("(APP) - received [%d] [%s]", code, b);
    linq_usbh_free(&usb);
}
