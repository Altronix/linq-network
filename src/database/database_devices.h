#ifndef DATABASE_DEVICES_H
#define DATABASE_DEVICES_H

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void database_devices_create_table(database_s* database, const char* name);

#ifdef __cplusplus
}
#endif
#endif
