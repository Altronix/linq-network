#ifndef DATABASE_ALERTS_H
#define DATABASE_ALERTS_H

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void database_alerts_create_table(database_s* database, const char* name);

#ifdef __cplusplus
}
#endif
#endif
