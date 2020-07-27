#include "device.h"

#define QUERY                                                                  \
    "SELECT device_id,product,prj_version,atx_version,web_version,mac "        \
    "FROM devices ORDER BY device_id "                                         \
    "LIMIT %d OFFSET %d"

#define QUERY_STATIC                                                           \
    "SELECT device_id,product,prj_version,atx_version,web_version,mac "        \
    "FROM devices ORDER BY device_id "                                         \
    "LIMIT 20 OFFSET 0"

int
device_open(database_s* db, device_s* d, uint32_t limit, uint32_t offset)
{
    char query[256];
    int l, err;
    memset(d, 0, sizeof(user_s));

    if (limit && offset) {
        l = snprintf(query, sizeof(query), QUERY, limit, offset);
    } else {
        l = snprintf(query, sizeof(query), QUERY_STATIC);
    }
    err = sqlite3_prepare_v2(db->db, query, l + 1, &d->stmt, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = device_next(d);
    if (!err) {
        return 0;
    } else {
        sqlite3_finalize(d->stmt);
        return -1;
    }
}

int
device_next(device_s* d)
{
    int err = sqlite3_step(d->stmt);
    if (err == SQLITE_ROW) {
        d->id = (const char*)sqlite3_column_text(d->stmt, 0);
        d->product = (const char*)sqlite3_column_text(d->stmt, 1);
        d->prj_version = (const char*)sqlite3_column_text(d->stmt, 2);
        d->atx_version = (const char*)sqlite3_column_text(d->stmt, 3);
        d->web_version = (const char*)sqlite3_column_text(d->stmt, 4);
        d->mac = (const char*)sqlite3_column_text(d->stmt, 5);
        return 0;
    } else {
        return DATABASE_DONE;
    }
}

void
device_close(device_s* d)
{
    sqlite3_finalize(d->stmt);
    memset(d, 0, sizeof(device_s));
}
