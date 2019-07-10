#include "device.h"

typedef struct device
{
    const zsock_t** sock_p;
    uint8_t router[256];
    char serial[64];
    char product[64];
    uint32_t birth;
    uint32_t uptime;
    uint32_t last_seen;
} device;

int
device_remove_serial(const char* serial)
{
    ((void)serial);
    return 0;
}

int
device_remove_product(const char* product)
{
    ((void)product);
    return 0;
}

uint32_t
device_count()
{
    return 0;
}
