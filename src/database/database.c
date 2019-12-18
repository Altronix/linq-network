#include "database.h"
#include "log.h"

void
database_init(database_s* d)
{
    memset(d, 0, sizeof(database_s));
    int err = sqlite3_open_v2(
        "file:test.db",
        &d->db,
        SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL);
    linq_netw_assert(err == SQLITE_OK);
    if (database_table_exists(d, "Alert")) {
        log_info("(DATA) [Alerts] Found!");
    } else {
        log_info("(DATA) [Alerts] Not found!");
        log_info("(DATA) [Alerts] Creating \"Alerts\" database");
    }
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);     // NOTE db can be busy, but we
    linq_netw_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
}

bool
database_table_exists(database_s* d, const char* table)
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
