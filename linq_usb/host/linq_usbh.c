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
static void device_free(device_s** dev_p);
MAP_INIT(device, device_s, device_free);

static void
device_free(device_s** dev_p)
{
    device_s* d = *dev_p;
    *dev_p = NULL;
    if (d->descriptors.config[0].config) {
        libusb_free_config_descriptor(d->descriptors.config[0].config);
    }
    libusb_close(d->handle);
    if (d->io) io_m5_free(&d->io);
    free(d);
}

static device_s*
device_init(libusb_device* d, struct libusb_device_descriptor descriptor)
{
    device_s* device = NULL;
    libusb_device_handle* handle;
    int err = libusb_open(d, &handle);
    uint8_t encoding[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 };
    if (!err && (device = malloc(sizeof(device_s)))) {
        // TODO this can be moved to io layer since it is device specific
        memset(device, 0, sizeof(device_s));
        device->device = d;
        device->descriptors.device = descriptor;
        device->handle = handle;
        err = libusb_get_config_descriptor(
            device->device, 0, &device->descriptors.config[0].config);
        if (err) { log_error("(USB) - cfg [%s]", libusb_strerror(err)); }
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iManufacturer,
            device->manufacturer,
            sizeof(device->manufacturer));
        if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iProduct,
            device->product,
            sizeof(device->product));
        if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
        err = libusb_get_string_descriptor_ascii(
            device->handle,
            descriptor.iSerialNumber,
            device->serial,
            sizeof(device->serial));
        if (err < 0) log_error("(USB) - str [%s]", libusb_strerror(err));
        for (int i = 0; i < 2; i++) {
            if (libusb_kernel_driver_active(device->handle, i)) {
                log_info("(USB) - detatching kernel driver [%d]", i);
                err = libusb_detach_kernel_driver(device->handle, i);
                if (err) log_error("(USB) - [%s]", libusb_strerror(err));
            }
            log_info("(USB) - claiming interface [%d]", i);
            err = libusb_claim_interface(device->handle, i);
            if (err) log_error("(USB) - [%s]", libusb_strerror(err));
        }
        // https://github.com/tytouf/libusb-cdc-example/blob/master/cdc_example.c
        err = libusb_control_transfer(
            device->handle, 0x21, 0x22, 0x01 | 0x02, 0, NULL, 0, 0);
        if (err < 0) log_error("USB) - ctr [%s]", libusb_strerror(err));
        err = libusb_control_transfer(
            device->handle, 0x21, 0x20, 0, 0, encoding, sizeof(encoding), 0);
        if (err < 0) log_error("USB) - ctr [%s]", libusb_strerror(err));
        device->io = io_m5_init(device->handle);
        if (!device->io) log_error("(USB) - io [%s]", "mem");
        // TODO - install io driver per product type
        //      - Optionally ? libusb_set_configuration( ...
        //      - Optionally ? libusb_claim_interface( ...
    } else {
        log_error("(USB) - open [%s]", err ? libusb_strerror(err) : "mem");
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
    device_s* device;
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
                device->descriptors.device.idVendor,
                device->descriptors.device.idProduct,
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
    device_s* d;
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
                    d = device_init(dev, desc);
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
linq_usbh_send_http_request(
    linq_usbh_s* usb,
    const char* serial,
    const char* meth,
    const char* path,
    const char* data,
    ...)
{
    device_s** d_p = device_map_get(usb->devices, serial);
    if (d_p) {
        device_s* d = *d_p;
        int err;
        va_list list;
        va_start(list, data);
        err = d->io->ops.vtx(d->io, meth, path, data, list);
        va_end(list);
        return err;
        // va_start(list, data);
        // err = libusb_bulk_transfer(
        //     (*d)->handle,
        //     2 | LIBUSB_ENDPOINT_OUT,
        //     (uint8_t*)"foo",
        //     3,
        //     &transfered,
        //     0);
        // log_info("(USB) - Sent [%d] bytes", transfered);
        // if (err < 0) {
        //     log_error("(USB) - Device send error [%s]",
        //     libusb_strerror(err)); log_error("(USB) - IO error [%s]",
        //     strerror(errno));
        // }
        // return err;
    } else {
        log_error("(USB) - Device not connected [%s]", serial);
        return -1;
    }
}
