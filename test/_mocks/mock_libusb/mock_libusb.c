#include "mock_libusb.h"
#include "containers.h"

typedef struct device_s
{
    struct libusb_device_descriptor desc;
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
spy_libusb_push_device(struct libusb_device_descriptor* desc)
{
    char key[8];
    device_s* device = malloc(sizeof(device_s));
    assert(device);
    memcpy(&device->desc, desc, sizeof(struct libusb_device_descriptor));
    snprintf(key, sizeof(key), "%d", devices_map_size(devices));
    devices_map_add(devices, key, &device);
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
    int* ptr;
    uint32_t sz = devices_map_size(devices);
    int* devices = malloc(sizeof(int*) * sz);
    memset(devices, 0, sizeof(int*) * sz);
    for (int i = 0; i < sz; i++) (devices)[i] = sz;
    *list_p = (libusb_device**)devices;
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
    char key[8];
    int idx = *(int*)dev;
    snprintf(key, sizeof(key), "%d", idx);
    d = devices_map_get(devices, key);
    if (d) {
        memcpy(desc, &(*d)->desc, sizeof(struct libusb_device_descriptor));
        return 0;
    } else {
        return -1;
    }
}
