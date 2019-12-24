#include "linq_netw_internal.h"
#include "routes.h"

#define QUERY                                                                  \
    "SELECT alert_id,who,what,site_id,time,mesg,device_id "                    \
    "FROM alerts ORDER BY time "                                               \
    "LIMIT %.*s OFFSET %.*s"

#define QUERY_STATIC                                                           \
    "SELECT alert_id,who,what,site_id,time,mesg,device_id "                    \
    "FROM alerts ORDER BY time "                                               \
    "LIMIT 50 OFFSET 0"

#define ALERT                                                                  \
    "{"                                                                        \
    "\"alert_id\":\"%s\","                                                     \
    "\"device_id\":\"%s\","                                                    \
    "\"who\":\"%s\","                                                          \
    "\"what\":\"%s\","                                                         \
    "\"site_id\":\"%s\","                                                      \
    "\"when\":\"%s\","                                                         \
    "\"mesg\":\"%s\""                                                          \
    "}"

void
route_alerts(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t _l,
    const char* _body)
{
    ((void)_l);
    ((void)_body);
    int err;
    uint32_t l;
    char b[LINQ_NETW_MAX_RESPONSE_SIZE];
    const char *count = NULL, *offset = NULL;
    uint32_t countl, offsetl;
    sqlite3_stmt* stmt;
    database_s* db = linq_netw_database(ctx->context);

    if (!(meth == HTTP_METHOD_GET)) {
        http_printf_json(ctx, 400, "{\"error\":\"Bad request\"}");
        return;
    }

    // Parse query params if provided then generate sql
    http_parse_query_str(ctx, "count", &count, &countl);
    http_parse_query_str(ctx, "offset", &offset, &offsetl);

    // TODO test should compare db statements with query string vs no query str
    if (count && offset && countl < 6 && offsetl < 6) {
        l = snprintf(b, sizeof(b), QUERY, countl, count, offsetl, offset);
    } else {
        l = snprintf(b, sizeof(b), QUERY_STATIC);
    }
    err = sqlite3_prepare_v2(db->db, b, l + 1, &stmt, NULL);
    linq_netw_assert(err == SQLITE_OK);

    // Convert database output to json
    l = snprintf(b, sizeof(b), "{\"alerts\":[");
    err = sqlite3_step(stmt);
    while (err == SQLITE_ROW && (l < sizeof(b))) {
        const char *alert_id = (const char*)sqlite3_column_text(stmt, 0),
                   *device_id = (const char*)sqlite3_column_text(stmt, 1),
                   *who = (const char*)sqlite3_column_text(stmt, 2),
                   *what = (const char*)sqlite3_column_text(stmt, 3),
                   *site = (const char*)sqlite3_column_text(stmt, 4),
                   *when = (const char*)sqlite3_column_text(stmt, 5),
                   *mesg = (const char*)sqlite3_column_text(stmt, 6);
        l += snprintf(
            &b[l],
            sizeof(b) - l,
            ALERT,
            alert_id,
            device_id,
            who,
            what,
            site,
            when,
            mesg);
        err = sqlite3_step(stmt);
        if (err == SQLITE_ROW && l < sizeof(b)) {
            l += snprintf(&b[l], sizeof(b) - l, ",");
        }
    }
    sqlite3_finalize(stmt);
    if (l < sizeof(b)) {
        l += snprintf(&b[l], sizeof(b) - l, "]}");
        http_printf_json(ctx, 200, b);
    } else {
        snprintf(b, sizeof(b), "{\"error\":\"Response too large\"}");
        http_printf_json(ctx, 400, b);
    }
}
