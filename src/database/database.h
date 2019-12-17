#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct database_s
    {
        sqlite3* db;
    } database_s;

    void database_init(database_s* d);
    void database_deinit(database_s* d);

#ifdef __cplusplus
}
#endif
#endif
