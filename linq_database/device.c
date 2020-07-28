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

void
device_close(device_s* d)
{
    sqlite3_finalize(d->stmt);
    memset(d, 0, sizeof(device_s));
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

int
device_insert_json(
    database_s* db,
    const char* serial,
    const char* json,
    uint32_t json_len)
{
    json_parser p;
    json_init(&p);
    jsontok t[64];
    const jsontok *product, *prj, *atx, *web, *mac;
    device_insert_s device;
    uint32_t count = json_parse(&p, json, json_len, t, 64);
    int err = -1;
    if (count > 0 && (product = json_delve(json, t, ".about.product")) &&
        (atx = json_delve(json, t, ".about.atxVersion")) &&
        (prj = json_delve(json, t, ".about.prjVersion")) &&
        (web = json_delve(json, t, ".about.webVersion")) &&
        (mac = json_delve(json, t, ".about.mac"))) {
        device.product = json_tok_value(json, product);
        device.prj_version = json_tok_value(json, prj);
        device.atx_version = json_tok_value(json, atx);
        device.web_version = json_tok_value(json, web);
        device.mac = json_tok_value(json, mac);
        err = device_insert(db, serial, &device);
    }
    return err;
}

int
device_insert(database_s* db, const char* serial, device_insert_s* d)
{

    int err = -1;
    char vals[128];
    const char* keys =
        "device_id,product,prj_version,atx_version,web_version,mac";
    uint32_t count, vlen, keylen = strlen(keys);

    // Print out sqlite format values
    if (d->product.p && d->prj_version.p && d->atx_version.p &&
        d->web_version.p && d->mac.p) {
        // clang-format off
        vlen = snprintf(
            vals,
            sizeof(vals),
            "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\"",
            (int)strlen(serial), serial,
            d->product.len,      d->product.p,     // product
            d->prj_version.len,  d->prj_version.p, // prjVersion
            d->atx_version.len,  d->atx_version.p, // atxVersion
            d->web_version.len,  d->web_version.p, // webVersion
            d->mac.len,          d->mac.p);        // mac
        // clang-format on

        err = database_insert_raw_n(db, "devices", keys, keylen, vals, vlen);
        return err;
    } else {
        return -1;
    }
}
