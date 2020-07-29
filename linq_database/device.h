#ifndef DEVICE_H_
#define DEVICE_H_

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int device_open(database_s*, device_s*, uint32_t, uint32_t);
    void device_close(device_s* a);
    int device_next(device_s* a);
    int device_insert(database_s* db, const char* serial, device_insert_s* d);
    int device_insert_json(
        database_s* db,
        const char* serial,
        const char*,
        uint32_t);

#ifdef __cplusplus
}
#endif
#endif

