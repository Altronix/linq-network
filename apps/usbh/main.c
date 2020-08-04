#include "linq_usbh.h"
#include "log.h"

int
main(int argc, char* argv[])
{
    char buffer[4096];
    uint32_t l = sizeof(buffer);
    linq_usbh_s usb;
    linq_usbh_init(&usb);
    linq_usbh_scan(&usb, buffer, &l);
    log_info("(USB) %.*s", l, buffer);
    linq_usbh_free(&usb);
}
