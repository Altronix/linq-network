#include "io.h"

void
io_free(io_s** io_p)
{
    io_s* d = *io_p;
    *io_p = NULL;
    if (d->desc_cfg) { libusb_free_config_descriptor(d->desc_cfg); }
    libusb_close(d->handle);
    free(d);
}
