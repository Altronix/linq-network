#ifndef DEVICE_H_
#define DEVICE_H_

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int device_open(database_s*, device_s*, uint32_t, uint32_t);
    int device_next(device_s* a);
    void device_close(device_s* a);

#ifdef __cplusplus
}
#endif
#endif

