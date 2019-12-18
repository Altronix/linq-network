#include "database.h"
#include "log.h"

#define DATABASE_DEVICES                                                       \
    "CREATE TABLE %s("                                                         \
    "device_id TEXT PRIMARY KEY,"                                              \
    "product TEXT,"                                                            \
    "prj_version TEXT,"                                                        \
    "atx_version INTEGER,"                                                     \
    "web_version TEXT,"                                                        \
    "mac TEXT"                                                                 \
    ");"

#define DATABASE_ALERTS                                                        \
    "CREATE TABLE %s("                                                         \
    "alert_id INTEGER PRIMARY KEY,"                                            \
    "who TEXT,"                                                                \
    "what TEXT,"                                                               \
    "site_id TEXT,"                                                            \
    "time INTEGER,"                                                            \
    "mesg TEXT,"                                                               \
    "name TEXT,"                                                               \
    "device_id INTEGER,"                                                       \
    "FOREIGN KEY(device_id) REFERENCES devices(device_id)"                     \
    ");"

static bool
row_exists(database_s* d, const char* table, const char* prop, const char* want)
{
    char stmt[128];
    sqlite3_stmt* sql;
    bool ret;
    int err, len = snprintf(
                 stmt,
                 sizeof(stmt),
                 "SELECT EXISTS(SELECT 1 FROM %s WHERE %s=%s LIMIT 1);",
                 table,
                 prop,
                 want);
    linq_netw_assert(len < 128);
    err = sqlite3_prepare_v2(d->db, stmt, len + 1, &sql, NULL);
    linq_netw_assert(err == SQLITE_OK);
    err = sqlite3_step(sql);
    ret = err == 1 ? true : false;
    sqlite3_finalize(sql);
    return ret;
}

static bool
table_exists(database_s* d, const char* table)
{
    char stmt[128];
    bool ret = false;
    sqlite3_stmt* result;
    int err,
        len = snprintf(
            stmt,
            sizeof(stmt),
            "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
            table);
    linq_netw_assert(len <= 128);
    err = sqlite3_prepare_v2(d->db, stmt, len + 1, &result, NULL);
    linq_netw_assert(err == SQLITE_OK);
    err = sqlite3_step(result);
    if (err == SQLITE_ROW) ret = true;
    sqlite3_finalize(result);
    return ret;
}

static void
create_table(database_s* d, const char* name, const char* database)
{
    char table[512];
    snprintf(table, sizeof(table), database, name);
    database_assert_command(d, table);
}

void
database_init(database_s* d)
{
    memset(d, 0, sizeof(database_s));

    // Open database
    int err = sqlite3_open_v2(
        "file:test.db",
        &d->db,
        SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL);
    linq_netw_assert(err == SQLITE_OK);
    ((void)err);

    // Enable FOREIGN_KEYS
    database_assert_command(d, "PRAGMA FOREIGN_KEYS = ON;");

    if (!table_exists(d, "devices")) {
        log_info("(DATA) \"devices\" database Not found! Creating database...");
        create_table(d, "devices", DATABASE_DEVICES);
        log_info("(DATA) \"devices\" database created success!");
    } else {
        log_info("(DATA) \"devices\" database found!");
    }

    if (!table_exists(d, "alerts")) {
        log_info("(DATA) \"alerts\" database Not found! Creating database...");
        create_table(d, "alerts", DATABASE_ALERTS);
        log_info("(DATA) \"alerts\" database created success!");
    } else {
        log_info("(DATA) \"alerts\" database found!");
    }
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);     // NOTE db can be busy, but we
    linq_netw_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
    ((void)err);
}

bool
database_row_exists(
    database_s* d,
    const char* table,
    const char* key,
    const char* val)
{
    return row_exists(d, table, key, val);
}
