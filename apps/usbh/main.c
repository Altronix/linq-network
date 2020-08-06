#include "linq_usbh.h"
#include "log.h"

int
main(int argc, char* argv[])
{
    linq_usbh_s usb;
    linq_usbh_init(&usb);
    linq_usbh_scan(&usb, 0x3333, 0x4444);
    linq_usbh_send_http_request(&usb, "N/A", "GET", "/api/v1/foo", NULL);
    linq_usbh_free(&usb);
}
