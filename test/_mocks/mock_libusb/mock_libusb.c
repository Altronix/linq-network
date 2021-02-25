#include "mock_libusb.h"
#include "common/containers.h"

typedef struct device_s
{
    char key[3];
    struct libusb_device_descriptor desc;
    char string[32];
} device_s;
MAP_INIT_W_FREE(devices, device_s);
static devices_map_s* devices = NULL;

void
spy_libusb_init()
{
    spy_libusb_free();
    devices = devices_map_create();
}

void
spy_libusb_free()
{
    if (devices) devices_map_destroy(&devices);
}

void
spy_libusb_push_device(struct libusb_device_descriptor* desc, const char* s)
{
    uint32_t sz = devices_map_size(devices);
    device_s* device = malloc(sizeof(device_s));
    assert(device);
    memset(device, 0, sizeof(device_s));
    memcpy(&device->desc, desc, sizeof(struct libusb_device_descriptor));
    snprintf(device->key, sizeof(device->key), "%d", sz);
    snprintf(device->string, sizeof(device->string), "%s", s);
    devices_map_add(devices, device->key, &device);
}

int
__wrap_libusb_set_option(libusb_context* ctx, enum libusb_option option, ...)
{
    return 0;
}

int
__wrap_libusb_init(libusb_context** ctx)
{
    return 0;
}

void
__wrap_libusb_exit(libusb_context* ctx)
{}

ssize_t
__wrap_libusb_get_device_list(libusb_context* ctx, libusb_device*** list_p)
{
    uint32_t sz = devices_map_size(devices);
    const char** ptr = *(const char***)list_p =
        malloc(sizeof(char*) * (sz + 1));
    assert(ptr);
    devices_iter iter;
    map_foreach(devices, iter)
    {
        if (map_has_key(devices, iter)) *ptr++ = map_key(devices, iter);
    }
    *ptr = NULL;
    return sz;
}

ssize_t
__wrap_libusb_free_device_list(libusb_device** list, int unref_devices)
{
    free(list);
    return 0;
}

int
__wrap_libusb_get_device_descriptor(
    libusb_device* dev,
    struct libusb_device_descriptor* desc)
{
    device_s** d;
    d = devices_map_get(devices, (char*)dev);
    if (d) {
        memcpy(desc, &(*d)->desc, sizeof(struct libusb_device_descriptor));
        return 0;
    } else {
        return -1;
    }
}

int
__wrap_libusb_get_config_descriptor(
    libusb_device* dev,
    uint8_t config,
    struct libusb_config_descriptor** desc)
{
    return 0;
}

int
__wrap_libusb_open(libusb_device* dev, libusb_device_handle** handle)
{
    device_s** d = devices_map_get(devices, (char*)dev);
    if (d) {
        *((char**)handle) = (*d)->key;
        return 0;
    } else {
        handle = NULL;
        return -1;
    }
}

void
__wrap_libusb_close(libusb_device_handle* handle)
{}

int
__wrap_libusb_get_string_descriptor_ascii(
    libusb_device_handle* handle,
    uint8_t idx,
    unsigned char* buffer,
    uint32_t len)
{
    device_s** d = devices_map_get(devices, (char*)handle);
    assert(d);
    return snprintf((char*)buffer, len, "%s", (*d)->string);
}

int
__wrap_libusb_control_transfer(
    libusb_device_handle* dev_handle,
    uint8_t request_type,
    uint8_t bRequest,
    uint16_t wValue,
    uint16_t wIndex,
    unsigned char* data,
    uint16_t wLength,
    unsigned int timeout)
{
    return 0;
}

int
__wrap_libusb_kernel_driver_active(
    libusb_device_handle* dev_handle,
    int interface_number)
{
    return 0;
}

int
__wrap_libusb_detach_kernel_driver(
    libusb_device_handle* dev_handle,
    int interface_number)
{
    return 0;
}

int
__wrap_libusb_attach_kernel_driver(
    libusb_device_handle* dev_handle,
    int interface_number)
{
    return 0;
}

int
__wrap_libusb_claim_interface(
    libusb_device_handle* dev_handle,
    int interface_number)
{
    return 0;
}

