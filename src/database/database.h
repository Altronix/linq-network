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
    bool database_table_exists(database_s* d, const char* table);

#ifdef __cplusplus
}
#endif
#endif
