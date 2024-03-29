#include "usbh.h"
#include "common/log.h"
#include "errno.h"
#include "io_m5.h"
#include "libusb-1.0/libusb.h"
#define usb_info(...) log_info("USB", __VA_ARGS__)
#define usb_warn(...) log_warn("USB", __VA_ARGS__)
#define usb_debug(...) log_debug("USB", __VA_ARGS__)
#define usb_trace(...) log_trace("USB", __VA_ARGS__)
#define usb_error(...) log_error("USB", __VA_ARGS__)
#define usb_fatal(...) log_fatal("USB", __VA_ARGS__)

typedef libusb_context usb_context;

static void
io_error_fn(node_s* n, void* ctx, int err)
{
    usbh_s* usb = ctx;
    const char* serial = device_serial(n);
    if (LIBUSB_ERROR_NO_DEVICE == err) {
        usb_error("device not available, removing device [%s]", serial);
        devices_remove(*usb->devices_p, serial);
    }
}

static io_callbacks_s callbacks = { .err = io_error_fn };

void
usbh_init(usbh_s* usb, device_map_s** devices_p)
{
    memset(usb, 0, sizeof(usbh_s));
    int err = libusb_init(&usb->context);
    usb->devices_p = devices_p;
    assert(err == 0);
    libusb_set_option(
        usb->context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
}

void
usbh_free(usbh_s* usb)
{
    libusb_exit(usb->context);
    memset(usb, 0, sizeof(usbh_s));
}

int
usbh_poll(usbh_s* usb, uint32_t ms)
{
    return 0;
}

uint32_t
usbh_device_count(usbh_s* usb)
{
    return devices_size(*usb->devices_p);
}

int
usbh_scan(usbh_s* usb, uint16_t vend, uint16_t prod)
{
    node_s* d;
    libusb_device **devs, *dev;
    const char* serial;
    int n = 0, i = 0;
    uint32_t count = libusb_get_device_list(usb->context, &devs);
    if (count > 0) {
        dev = devs[i];
        while (dev) {
            struct libusb_device_descriptor desc;
            int err = libusb_get_device_descriptor(dev, &desc);
            if (err == 0) {
                usb_info("scan [%d/%d]", desc.idVendor, desc.idProduct);
                if (desc.idVendor == vend && desc.idProduct == prod) {
                    d = io_m5_init(dev, desc, &callbacks, usb);
                    if (d) {
                        ++n;
                        usb_info("disc [%s]", d->serial);
                        serial = (const char*)d->serial;
                        if (!devices_get(*usb->devices_p, serial)) {
                            devices_add(*usb->devices_p, serial, &d);
                        } else {
                            io_m5_free(&d);
                        }
                    }
                }
            }
            dev = devs[++i];
        }
        libusb_free_device_list(devs, 1);
    }

    return n;
}
