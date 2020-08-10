#include "linq_usbh.h"
#include "errno.h"
#include "io_m5.h"
#include "libusb-1.0/libusb.h"
#include "log.h"

#define SCAN_FMT                                                               \
    "\"%s\":{"                                                                 \
    "\"idVendor\":%d,"                                                         \
    "\"idProduct\":%d,"                                                        \
    "\"manufacturer\":\"%s\","                                                 \
    "\"product\":\"%s\""                                                       \
    "}"

typedef libusb_context usb_context;
MAP_INIT(device, io_s, io_free);

void
linq_usbh_init(linq_usbh_s* usb)
{
    memset(usb, 0, sizeof(linq_usbh_s));
    int err = libusb_init(&usb->context);
    usb->devices = device_map_create();
    assert(err == 0);
    libusb_set_option(
        usb->context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
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
linq_usbh_print_devices(linq_usbh_s* usb, char* b, uint32_t l)
{
    uint32_t n = device_map_size(usb->devices), sz = l;
    l = 1;
    if (sz) *b = '{';
    io_s* device;
    device_iter iter;
    map_foreach(usb->devices, iter)
    {
        if (map_has_key(usb->devices, iter)) {
            device = map_val(usb->devices, iter);
            l += snprintf(
                &b[l],
                sz - l,
                SCAN_FMT,
                device->serial,
                device->desc_dev.idVendor,
                device->desc_dev.idProduct,
                device->manufacturer,
                device->product);
            if (--n) {
                if (l < sz) b[(l)++] = ',';
            }
        }
    }
    if (l < sz) b[(l)++] = '}';
    if (l < sz) b[(l)] = '\0';
    return l;
}

int
linq_usbh_scan(linq_usbh_s* usb, uint16_t vend, uint16_t prod)
{
    io_s* d;
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
                log_info("(USB) - scan [%d/%d]", desc.idVendor, desc.idProduct);
                if (desc.idVendor == vend && desc.idProduct == prod) {
                    // TODO this should be opaque init function
                    d = io_m5_init(dev, desc);
                    if (d) {
                        log_info("(USB) - disc [%d] [%s]", n + 1, d->serial);
                        serial = (const char*)d->serial;
                        device_map_add(usb->devices, serial, &d);
                        ++n;
                    }
                }
            }
            dev = devs[++i];
        }
    }

    libusb_free_device_list(devs, 1);
    return n;
}

int
linq_usbh_send_http_request_sync(
    linq_usbh_s* usb,
    const char* serial,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    int err = -1;
    io_s** d_p = device_map_get(usb->devices, serial);
    if (d_p) {
        io_s* d = *d_p;
        va_list list;
        va_start(list, data);
        err = d->ops.vtx_sync(d, meth, path, data, list);
        va_end(list);
    }
    return err;
}

int
linq_usbh_recv_http_response_sync(
    linq_usbh_s* usb,
    const char* serial,
    uint16_t* code,
    char* buff,
    uint32_t l)
{
    int err = -1;
    io_s** d_p = device_map_get(usb->devices, serial);
    if (d_p) {
        io_s* d = *d_p;
        err = d->ops.rx_sync(d, code, buff, l);
    }
    return err;
}
