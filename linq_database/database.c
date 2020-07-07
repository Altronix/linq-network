#include "database.h"
#include "jsmn/jsmn_helpers.h"
#include "log.h"

#define DATABASE_DEVICES                                                       \
    "CREATE TABLE %s("                                                         \
    "device_id TEXT PRIMARY KEY,"                                              \
    "product TEXT,"                                                            \
    "prj_version TEXT,"                                                        \
    "atx_version TEXT,"                                                        \
    "web_version TEXT,"                                                        \
    "mac TEXT"                                                                 \
    ");"

#define DATABASE_ALERTS                                                        \
    "CREATE TABLE %s("                                                         \
    "alert_id TEXT PRIMARY KEY,"                                               \
    "who TEXT,"                                                                \
    "what TEXT,"                                                               \
    "site_id TEXT,"                                                            \
    "time INTEGER,"                                                            \
    "mesg TEXT,"                                                               \
    "name TEXT,"                                                               \
    "device_id TEXT,"                                                          \
    "FOREIGN KEY(device_id) REFERENCES devices(device_id)"                     \
    ");"

#define DATABASE_USERS                                                         \
    "CREATE TABLE %s("                                                         \
    "user_id TEXT PRIMARY KEY,"                                                \
    "user TEXT,"                                                               \
    "pass TEXT,"                                                               \
    "salt BLOB,"                                                               \
    "role INTEGER"                                                             \
    ");"

#define database_assert_command(__database, __command)                         \
    do {                                                                       \
        int err;                                                               \
        sqlite3_stmt* sql;                                                     \
        err = sqlite3_prepare_v2(__database->db, __command, -1, &sql, NULL);   \
        linq_network_assert(err == SQLITE_OK);                                 \
        err = sqlite3_step(sql);                                               \
        linq_network_assert(err == SQLITE_DONE);                               \
        sqlite3_finalize(sql);                                                 \
    } while (0);

static bool
row_exists(database_s* d, const char* table, const char* prop, const char* want)
{
    char stmt[512];
    sqlite3_stmt* sql;
    bool ret = true;
    int err, len = snprintf(
                 stmt,
                 sizeof(stmt),
                 "SELECT EXISTS(SELECT 1 FROM %s WHERE %s=%s LIMIT 1);",
                 table,
                 prop,
                 want);
    linq_network_assert(len + 1 < 512);
    err = sqlite3_prepare_v2(d->db, stmt, len + 1, &sql, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = sqlite3_step(sql);
    if (err == SQLITE_ROW) {
        ret = sqlite3_column_int(sql, 0) ? true : false;
    } else {
        ret = false;
    }
    sqlite3_finalize(sql);
    return ret;
}

static bool
table_exists(database_s* d, const char* table)
{
    char stmt[512];
    bool ret = false;
    sqlite3_stmt* result;
    int err,
        len = snprintf(
            stmt,
            sizeof(stmt),
            "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
            table);
    linq_network_assert(len <= 512);
    err = sqlite3_prepare_v2(d->db, stmt, len + 1, &result, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = sqlite3_step(result);
    if (err == SQLITE_ROW) ret = true;
    sqlite3_finalize(result);
    return ret;
}

static int
row_insert(
    database_s* d,
    const char* table,
    const char* keys,
    uint32_t keys_len,
    const char* vals,
    uint32_t vals_len)
{
    char stmt[2048 + 512 + 256]; // TODO - if len greater than stack use malloc
    ((void)keys_len);            // TODO - if len greater than stack use malloc
    ((void)vals_len);            // TODO - if len greater than stack use malloc
    sqlite3_stmt* sql;
    int err, n = snprintf(
                 stmt,
                 sizeof(stmt),
                 "INSERT INTO %s(%s) VALUES(%s);",
                 table,
                 keys,
                 vals);
    linq_network_assert(n + 1 <= sizeof(stmt));
    err = sqlite3_prepare_v2(d->db, stmt, n + 1, &sql, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = sqlite3_step(sql);
    sqlite3_finalize(sql);
    return err == SQLITE_DONE ? 0 : -1;
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
    struct
    {
        const char* table;
        const char* schema;
    } db[NUM_DATABASES] = { { .table = "devices", .schema = DATABASE_DEVICES },
                            { .table = "alerts", .schema = DATABASE_ALERTS },
                            { .table = "users", .schema = DATABASE_USERS } };
    memset(d, 0, sizeof(database_s));

    // Open database
    int err = sqlite3_open_v2(
        "file:test.db",
        &d->db,
        SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL);
    linq_network_assert(err == SQLITE_OK);
    ((void)err);

    // Enable FOREIGN_KEYS
    database_assert_command(d, "PRAGMA FOREIGN_KEYS = ON;");

    // Create databases
    for (int i = 0; i < NUM_DATABASES; i++) {
        if (!table_exists(d, db[i].table)) {
            log_info(
                "(DATA) %s database not found! Creating %s database...",
                db[i].table,
                db[i].table);
            create_table(d, db[i].table, db[i].schema);
        } else {
            log_info("(DATA) %s database found!", db[i].table);
        }
    }
}

void
database_deinit(database_s* d)
{
    int err = sqlite3_close(d->db);        // NOTE db can be busy, but we
    linq_network_assert(err == SQLITE_OK); // shouldn't be if shutdown properly
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

bool
database_row_exists_str(
    database_s* d,
    const char* table,
    const char* key,
    const char* val)
{
    char val_str[256];
    snprintf(val_str, sizeof(val_str), "\"%s\"", val);
    return row_exists(d, table, key, val_str);
}

bool
database_row_exists_mem(
    database_s* d,
    const char* table,
    const char* key,
    const char* val,
    uint32_t val_len)
{
    char val_str[256];
    snprintf(val_str, sizeof(val_str), "\"%.*s\"", val_len, val);
    return row_exists(d, table, key, val_str);
}

int
database_count(database_s* d, const char* table)
{
    char stmt[256];
    sqlite3_stmt* sql;
    int ret = -1, err,
        n = snprintf(stmt, sizeof(stmt), "SELECT COUNT(*) FROM %s;", table);
    linq_network_assert(n + 1 <= sizeof(stmt));
    err = sqlite3_prepare_v2(d->db, stmt, n + 1, &sql, NULL);
    linq_network_assert(err == SQLITE_OK);
    err = sqlite3_step(sql);
    if (err == SQLITE_ROW) { ret = sqlite3_column_int(sql, 0) ? true : false; }
    sqlite3_finalize(sql);
    return ret;
}

int
database_insert(database_s* d, const char* table, int n_columns, ...)
{
    char keys[512];  // TODO
    char vals[2048]; // TODO
    const char *key, *val;
    int n, sk = 0, sv = 0;
    va_list list;
    n = n_columns << 1;

    va_start(list, n_columns);
    while (n >= 2) {
        n -= 2;
        key = va_arg(list, const char*);
        val = va_arg(list, const char*);
        if (n) {
            sk += snprintf(&keys[sk], sizeof(keys) - sk, "%s,", key);
            sv += snprintf(&vals[sv], sizeof(vals) - sv, "%s,", val);
        } else {
            sk += snprintf(&keys[sk], sizeof(keys) - sk, "%s", key);
            sv += snprintf(&vals[sv], sizeof(vals) - sv, "%s", val);
        }
    }
    va_end(list);
    return row_insert(d, table, keys, sk, vals, sv);
}

int
database_insert_n(database_s* d, const char* table, int n_columns, ...)
{
    char keys[512];  // TODO
    char vals[2048]; // TODO MAX column * MAX_TEXT + sizeof INSERT...
    const char *key, *val;
    int val_len, n, sk = 0, sv = 0;
    va_list list;
    n = n_columns * 3;

    va_start(list, n_columns);
    while (n >= 3) {
        n -= 3;
        key = va_arg(list, const char*);
        val = va_arg(list, const char*);
        val_len = va_arg(list, int);
        linq_network_assert(sizeof(vals) - sv > sv + val_len + 1);
        if (n) {
            sk += snprintf(&keys[sk], sizeof(keys) - sk, "%s,", key);
            memcpy(&vals[sv], val, val_len);
            vals[sv + val_len] = ',';
            sv += (val_len + 1);
        } else {
            sk += snprintf(&keys[sk], sizeof(keys) - sk, "%s", key);
            memcpy(&vals[sv], val, val_len);
            vals[sv + val_len] = '\0';
            sv += val_len;
        }
    }
    va_end(list);

    return row_insert(d, table, keys, sk, vals, sv);
}

int
database_insert_raw(
    database_s* d,
    const char* table,
    const char* keys,
    const char* vals)
{
    return row_insert(d, table, keys, strlen(keys), vals, strlen(vals));
}

int
database_insert_raw_n(
    database_s* d,
    const char* table,
    const char* keys,
    uint32_t keys_len,
    const char* vals,
    uint32_t vals_len)
{
    return row_insert(d, table, keys, keys_len, vals, vals_len);
}

int
database_insert_device_from_json(
    database_s* db,
    const char* serial,
    const char* json,
    uint32_t json_len)
{
    int err = -1;
    jsmn_value sid, product, prj_version, atx_version, web_version, mac;
    char vals[512];
    const char* keys =
        "device_id,product,prj_version,atx_version,web_version,mac";
    uint32_t keylen = strlen(keys), count;
    jsmntok_t t[64];

    log_info("(DATA) [%.6s...] Adding device to database...", serial);
    // clang-format off
    count = jsmn_parse_tokens_path(
        "/about",
        t,
        64,
        json,
        json_len,
        6,
        "sid",       &sid,
        "product",   &product,
        "prjVersion",&prj_version,
        "atxVersion",&atx_version,
        "webVersion",&web_version,
        "mac",       &mac);
    // clang-format on
    if (count == 6) {
        // clang-format off
        uint32_t vallen = snprintf(
            vals,
            sizeof(vals),
            "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\"",
            sid.len,         sid.p,
            product.len,     product.p,
            prj_version.len, prj_version.p,
            atx_version.len, atx_version.p,
            web_version.len, web_version.p,
            mac.len,         mac.p);
        // clang-format on
        err = database_insert_raw_n(db, "devices", keys, keylen, vals, vallen);
        log_debug("(DATA) Database device insert result (%d)", err);
    } else {
        log_debug("(DATA) Heartbeat parser error");
    }
    return err;
}

int
database_insert_alert(database_s* db, const char* serial, jsmn_value a[])
{
    int err = -1;
    char vals[512];
    const char* keys = "alert_id,who,what,site_id,time,mesg,name,device_id";
    uint32_t count, vlen, keylen = strlen(keys);
    jsmntok_t t[64];
    char uuid[33];
    sys_uuid(uuid);

    // Print out sqlite format values
    // clang-format off
    vlen = snprintf(
        vals,
        sizeof(vals),
        "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%.*s,\"%.*s\",\"%.*s\",\"%.*s\"",
        32,       uuid,
        a[0].len, a[0].p, // who
        a[1].len, a[1].p, // what
        a[2].len, a[2].p, // where
        a[3].len, a[3].p, // when
        a[4].len, a[4].p, // mesg
        a[5].len, a[5].p, // name
        (int)strlen(serial), serial);
    // clang-format on

    err = database_insert_raw_n(db, "alerts", keys, keylen, vals, vlen);
    return err;
}
