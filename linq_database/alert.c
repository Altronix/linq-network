#include "alert.h"

#define QUERY                                                                  \
    "SELECT alert_id,who,what,site_id,time,mesg,name,device_id "               \
    "FROM alerts ORDER BY time "                                               \
    "LIMIT %d OFFSET %d"

#define QUERY_STATIC                                                           \
    "SELECT alert_id,who,what,site_id,time,mesg,name,device_id "               \
    "FROM alerts ORDER BY time "                                               \
    "LIMIT 50 OFFSET 0"

int
alert_open(database_s* db, alert_s* a, uint32_t limit, uint32_t offset)
{
    char query[256];
    int l, err;
    memset(a, 0, sizeof(user_s));

    if (limit && offset) {
        l = snprintf(query, sizeof(query), QUERY, limit, offset);
    } else {
        l = snprintf(query, sizeof(query), QUERY_STATIC);
    }
    err = sqlite3_prepare_v2(db->db, query, l + 1, &a->stmt, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = alert_next(a);
    if (!err) {
        return 0;
    } else {
        sqlite3_finalize(a->stmt);
        return -1;
    }
}

void
alert_close(alert_s* a)
{
    sqlite3_finalize(a->stmt);
    memset(a, 0, sizeof(alert_s));
}

int
alert_next(alert_s* a)
{
    int err = sqlite3_step(a->stmt);
    if (err == SQLITE_ROW) {
        a->id = (const char*)sqlite3_column_text(a->stmt, 0);
        a->who = (const char*)sqlite3_column_text(a->stmt, 1);
        a->what = (const char*)sqlite3_column_text(a->stmt, 2);
        a->site = (const char*)sqlite3_column_text(a->stmt, 3);
        a->time = sqlite3_column_int(a->stmt, 4);
        a->mesg = (const char*)sqlite3_column_text(a->stmt, 5);
        a->name = (const char*)sqlite3_column_text(a->stmt, 6);
        a->device = (const char*)sqlite3_column_text(a->stmt, 7);
        return 0;
    } else {
        return DATABASE_DONE;
    }
}

int
alert_insert(database_s* db, const char* serial, alert_insert_s* a)
{

    int err = -1;
    char vals[128];
    const char* keys = "alert_id,who,what,site_id,time,mesg,device_id";
    uint32_t count, vlen, keylen = strlen(keys);
    char uuid[33];
    sys_uuid(uuid);

    // Print out sqlite format values
    if (a->who.p && a->what.p && a->site.p && a->time.p && a->mesg.p) {
        // clang-format off
        vlen = snprintf(
            vals,
            sizeof(vals),
            "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%.*s,\"%.*s\",\"%.*s\"",
            32,       uuid,
            a->who.len, a->who.p, // who
            a->what.len, a->what.p, // what
            a->site.len, a->site.p, // where
            a->time.len, a->time.p, // when
            a->mesg.len, a->mesg.p, // mesg
            (int)strlen(serial), serial);
        // clang-format on

        err = database_insert_raw_n(db, "alerts", keys, keylen, vals, vlen);
        return err;
    } else {
        return -1;
    }
}
