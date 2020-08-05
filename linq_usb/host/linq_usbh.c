#include "linq_usbh.h"
#include "libusb-1.0/libusb.h"
#include "log.h"

#define LOG_SCAN "(USB) - Device detected [0x%d/0x%d]"
#define LOG_FOUND "(USB) - Scan found [%d] [serial: %s]"
#define LOG_FAIL_STRING "(USB) - Device failed to get string descriptor [%s]"
#define LOG_FAIL_OPEN "(USB) - Device failed to open [%s]"

#define SCAN_FMT                                                               \
    "{"                                                                        \
    "\"idProduct\":%d,"                                                        \
    "\"idVendor\":%d,"                                                         \
    "\"iManufacturer\":\"%d\","                                                \
    "\"iProduct\":\"%d\","                                                     \
    "\"iSerialNumber\":\"%d\""                                                 \
    "}"

typedef libusb_context usb_context;
static void device_free(device_s** dev_p);
MAP_INIT(device, device_s, device_free);

static void
device_free(device_s** dev_p)
{
    device_s* d = *dev_p;
    *dev_p = NULL;
    libusb_close(d->handle);
    free(d);
}

static device_s*
device_init(libusb_device* d, struct libusb_device_descriptor descriptor)
{
    device_s* device = NULL;
    libusb_device_handle* handle;
    int err = libusb_open(d, &handle);
    if (!err && (device = malloc(sizeof(device_s)))) {
        memset(device, 0, sizeof(device_s));
        device->device = d;
        device->descriptor = descriptor;
        device->handle = handle;
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iManufacturer,
            device->manufacturer,
            sizeof(device->manufacturer));
        if (err < 0) log_error(LOG_FAIL_STRING, "iManufacturer");
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iProduct,
            device->product,
            sizeof(device->product));
        if (err < 0) log_error(LOG_FAIL_STRING, "iProduct");
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iSerialNumber,
            device->serial,
            sizeof(device->serial));
        if (err < 0) log_error(LOG_FAIL_STRING, "iManufacturer");
    } else {
        log_error(LOG_FAIL_OPEN, err ? libusb_strerror(err) : "No memory");
    }
    return device;
}

void
linq_usbh_init(linq_usbh_s* usb)
{
    memset(usb, 0, sizeof(linq_usbh_s));
    int err = libusb_init(&usb->context);
    usb->devices = device_map_create();
    assert(err == 0);
}

void
linq_usbh_free(linq_usbh_s* usb)
{
    device_map_destroy(&usb->devices);
    libusb_exit(usb->context);
    memset(usb, 0, sizeof(linq_usbh_s));
}

uint32_t
linq_usbh_device_count(linq_usbh_s* usb)
{
    return device_map_size(usb->devices);
}

int
linq_usbh_scan(linq_usbh_s* usb, uint16_t vend, uint16_t prod)
{
    device_s* d;
    libusb_device **devs, *dev;
    int n = 0, i = 0;
    uint32_t count = libusb_get_device_list(usb->context, &devs);
    if (count > 0) {
        dev = devs[i];
        while (dev) {
            struct libusb_device_descriptor desc;
            int err = libusb_get_device_descriptor(dev, &desc);
            if (err == 0) {
                log_info(LOG_SCAN, desc.idVendor, desc.idProduct);
                if (desc.idVendor == vend && desc.idProduct == prod) {
                    d = device_init(dev, desc);
                    if (d) {
                        log_info(LOG_FOUND, n, d->serial);
                        device_map_add(
                            usb->devices, (const char*)d->serial, &d);
                        ++n;
                    }
                }
            }
            dev = devs[++i];
        }
    }
    /*
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
                    desc.iManufacturer,
                    desc.iProduct,
                    desc.iSerialNumber);
                dev = devs[++i];
                if (dev) {
                    if (*l < sz) b[(*l)++] = ',';
                }
            }
        }
    }
    if (*l < sz) b[(*l)++] = ']';
    if (*l < sz) b[(*l)++] = '\0';
    */
    libusb_free_device_list(devs, 1);
    return n;
}
