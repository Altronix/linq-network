#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_DATABASES 3

    typedef struct database_s
    {
        sqlite3* db;
    } database_s;

    void database_init(database_s* d);
    void database_deinit(database_s* d);
    bool database_row_exists(
        database_s* d,
        const char* table,
        const char* key,
        const char* val);
    bool database_row_exists_str(
        database_s* d,
        const char* table,
        const char* key,
        const char* val);
    int database_insert(database_s* d, const char* table, int n_columns, ...);
    int database_insert_n(database_s* d, const char* table, int n_columns, ...);
    int database_insert_raw(database_s*, const char*, const char*, const char*);
    int database_insert_raw_n(
        database_s* d,
        const char* table,
        const char* keys,
        uint32_t keys_len,
        const char* vals,
        uint32_t vals_len);

#ifdef __cplusplus
}
#endif
#endif
