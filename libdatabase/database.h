#ifndef DATABASE_H
#define DATABASE_H

#include "json.h"
#include "sqlite3.h"
#include "sys.h"

// clang-format off
#if defined _WIN32
#  if defined LINQ_DATABASE_STATIC
#    define LINQ_DATABASE_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_DATABASE_EXPORT __declspec(dllexport)
#  else
#    define LINQ_DATABASE_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_DATABASE_EXPORT
#endif
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_DATABASES 3
#define DATABASE_DONE -2

    typedef sqlite3_stmt handle;

    typedef struct database_s
    {
        sqlite3* db;
    } database_s;

    typedef struct user_s
    {
        const char* id;
        const char* user;
        const char* pass;
        const char* salt;
        uint32_t role;
        handle* stmt;
    } user_s;

    typedef struct alert_s
    {
        const char* id;
        const char* who;
        const char* what;
        const char* site;
        uint32_t time;
        const char* mesg;
        const char* name;
        const char* product;
        const char* device;
        handle* stmt;
    } alert_s;

    typedef struct alert_insert_s
    {
        json_value id;
        json_value who;
        json_value what;
        json_value site;
        json_value time;
        json_value mesg;
        json_value name;
        json_value product;
        json_value device;
    } alert_insert_s;

    typedef struct device_s
    {
        const char* id;
        const char* product;
        const char* prj_version;
        const char* atx_version;
        const char* web_version;
        const char* mac;
        handle* stmt;
    } device_s;

    typedef struct device_insert_s
    {
        json_value id;
        json_value product;
        json_value prj_version;
        json_value atx_version;
        json_value web_version;
        json_value mac;
    } device_insert_s;

    LINQ_DATABASE_EXPORT void database_init(database_s* d);
    LINQ_DATABASE_EXPORT void database_deinit(database_s* d);
    LINQ_DATABASE_EXPORT bool database_row_exists(
        database_s* d,
        const char* table,
        const char* key,
        const char* val);
    LINQ_DATABASE_EXPORT bool database_row_exists_str(
        database_s* d,
        const char* table,
        const char* key,
        const char* val);
    LINQ_DATABASE_EXPORT bool database_row_exists_mem(
        database_s* d,
        const char* table,
        const char* key,
        const char* val,
        uint32_t val_len);
    LINQ_DATABASE_EXPORT int database_count(database_s* d, const char* table);
    LINQ_DATABASE_EXPORT int
    database_insert(database_s* d, const char* table, int n_columns, ...);
    LINQ_DATABASE_EXPORT int
    database_insert_n(database_s* d, const char* table, int n_columns, ...);
    LINQ_DATABASE_EXPORT int
    database_insert_raw(database_s*, const char*, const char*, const char*);
    LINQ_DATABASE_EXPORT int database_insert_raw_n(
        database_s* d,
        const char* table,
        const char* keys,
        uint32_t keys_len,
        const char* vals,
        uint32_t vals_len);

    LINQ_DATABASE_EXPORT int
    database_user_open(database_s*, user_s* u, const char*);
    LINQ_DATABASE_EXPORT void database_user_close(user_s*);
    LINQ_DATABASE_EXPORT int
    database_alert_open(database_s*, alert_s*, uint32_t limit, uint32_t offset);
    LINQ_DATABASE_EXPORT void database_alert_close(alert_s*);
    LINQ_DATABASE_EXPORT int database_alert_next(alert_s*);
    LINQ_DATABASE_EXPORT int
    database_alert_insert(database_s*, const char*, alert_insert_s*);
    LINQ_DATABASE_EXPORT int database_device_open(
        database_s*,
        device_s*,
        uint32_t limit,
        uint32_t offset);
    LINQ_DATABASE_EXPORT void database_device_close(device_s*);
    LINQ_DATABASE_EXPORT int database_device_next(device_s*);
    LINQ_DATABASE_EXPORT int database_device_insert(
        database_s* db,
        const char* serial,
        device_insert_s* d);
    LINQ_DATABASE_EXPORT int database_device_insert_json(
        database_s* db,
        const char* serial,
        const char* json,
        uint32_t json_len);

#ifdef __cplusplus
}
#endif
#endif
