#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include "sys.h"

#define database_assert_command(__database, __command)                         \
    do {                                                                       \
        int err;                                                               \
        sqlite3_stmt* sql;                                                     \
        err = sqlite3_prepare_v2(__database->db, __command, -1, &sql, NULL);   \
        linq_netw_assert(err == SQLITE_OK);                                    \
        err = sqlite3_step(sql);                                               \
        linq_netw_assert(err == SQLITE_DONE);                                  \
        sqlite3_finalize(sql);                                                 \
    } while (0);

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
