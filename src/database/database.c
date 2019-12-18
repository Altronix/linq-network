#include "database.h"
#include "database_alerts.h"
#include "database_devices.h"
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
        log_info("(DATA) [devices] Not found!");
        log_info("(DATA) [devices] Creating \"devices\" database...");
        database_devices_create_table(d, "devices");
        log_info("(DATA) [devices] Database created success!");
    } else {
        log_info("(DATA) [devices] Database found!");
    }

    if (!table_exists(d, "alerts")) {
        log_info("(DATA) [alerts] Not found!");
        log_info("(DATA) [alerts] Creating \"alerts\" database...");
        database_alerts_create_table(d, "alerts");
        log_info("(DATA) [alerts] Database created success!");
    } else {
        log_info("(DATA) [alerts] Database found!");
    }
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);     // NOTE db can be busy, but we
    linq_netw_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
    ((void)err);
}

