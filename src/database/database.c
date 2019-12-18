#include "database.h"
#include "log.h"

static bool
table_exists(database_s* d, const char* table)
{
    char stmt[128];
    bool ret = false;
    sqlite3_stmt* result;
    int err,
        ln = snprintf(
            stmt,
            sizeof(stmt),
            "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
            table);
    linq_netw_assert(ln <= 128);
    err = sqlite3_prepare_v2(d->db, stmt, ln + 1, &result, NULL);
    linq_netw_assert(err == SQLITE_OK);
    err = sqlite3_step(result);
    if (err == SQLITE_ROW) ret = true;
    sqlite3_finalize(result);
    return ret;
}
void
database_init(database_s* d)
{
    memset(d, 0, sizeof(database_s));
    const char* enable_keys = "PRAGMA FOREIGN_KEYS = ON;";
    sqlite3_stmt* sql;

    // Open database
    int err = sqlite3_open_v2(
        "file:test.db",
        &d->db,
        SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL);
    linq_netw_assert(err == SQLITE_OK);

    // Enable FOREIGN_KEYS
    err = sqlite3_prepare_v2(d->db, enable_keys, -1, &sql, NULL);
    linq_netw_assert(err == SQLITE_OK);
    err = sqlite3_step(sql);
    linq_netw_assert(err == SQLITE_DONE);
    sqlite3_finalize(sql);

    // TODO create databases if they do not exist
    /*
    if (database_table_exists(d, "Alert")) {
        log_info("(DATA) [Alerts] Found!");
    } else {
        log_info("(DATA) [Alerts] Not found!");
        log_info("(DATA) [Alerts] Creating \"Alerts\" database");
    }
    */
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);     // NOTE db can be busy, but we
    linq_netw_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
    ((void)err);
}

