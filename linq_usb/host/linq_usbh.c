#include "linq_usbh.h"
#include "libusb-1.0/libusb.h"

#define SCAN_FMT                                                               \
    "{"                                                                        \
    "\"idProduct\":%d,"                                                        \
    "\"idVendor\":%d,"                                                         \
    "\"iManufacturer\":\"%s\","                                                \
    "\"iProduct\":\"%s\","                                                     \
    "\"iSerialNumber\":\"%s\""                                                 \
    "}"

typedef libusb_context usb_context;

void
linq_usbh_init(linq_usbh_s* usb)
{
    memset(usb, 0, sizeof(linq_usbh_s));
    int err = libusb_init(&usb->context);
    assert(err == 0);
}

void
linq_usbh_free(linq_usbh_s* usb)
{
    libusb_exit(usb->context);
    memset(usb, 0, sizeof(linq_usbh_s));
}

int
linq_usbh_scan(linq_usbh_s* usb, char* b, uint32_t* l)
{
    libusb_device **devs, *dev;
    int i = 0;
    uint32_t sz = *l, count = libusb_get_device_list(usb->context, &devs);
    *l = 1;
    if (sz) *b = '[';
    if (count > 0) {
        dev = devs[i];
        while (dev) {
            struct libusb_device_descriptor desc;
            int err = libusb_get_device_descriptor(dev, &desc);
            if (err == 0) {
                *l += snprintf(
                    &b[*l],
                    *l - sz,
                    SCAN_FMT,
                    desc.idProduct,
                    desc.idVendor,
                    "TODO Manufacturer",
                    "TODO Product",
                    "TODO SerialNumber");
                dev = devs[++i];
                if (dev) {
                    if (*l < sz) b[(*l)++] = ',';
                }
            }
        }
    }
    if (*l < sz) b[(*l)++] = ']';
    if (*l < sz) b[(*l)++] = '\0';
    libusb_free_device_list(devs, 1);
    return *l < sz ? 0 : -1;
}
