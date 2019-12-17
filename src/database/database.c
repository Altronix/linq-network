#include "database.h"

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
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);     // NOTE db can be busy, but we
    linq_netw_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
}

