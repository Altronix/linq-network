#include "routes.h"
#include "sys.h"

#define QUERY                                                                  \
    "SELECT device_id,product,prj_version,atx_version "                        \
    "FROM devices ORDER BY device_id "                                         \
    "LIMIT %.*s OFFSET %.*s"

#define QUERY_STATIC                                                           \
    "SELECT device_id,product,prj_version,atx_version "                        \
    "FROM devices ORDER BY device_id "                                         \
    "LIMIT 20 OFFSET 0"

#define DEVICE                                                                 \
    "\"%s\":{"                                                                 \
    "\"product\":\"%s\","                                                      \
    "\"prj_version\":\"%s\","                                                  \
    "\"atx_version\":\"%s\""                                                   \
    "}"

void
route_devices(
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
    database_s* db = &((http_s*)ctx->context)->db;

    if (!(meth == HTTP_METHOD_GET)) {
        http_printf_json(
            ctx->curr_connection, 400, "{\"error\":\"Bad request\"}");
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
    linq_network_assert(err == SQLITE_OK);

    // Convert database output to json
    l = snprintf(b, sizeof(b), "{\"devices\":{");
    err = sqlite3_step(stmt);
    while (err == SQLITE_ROW && (l < sizeof(b))) {
        const char *sid = (const char*)sqlite3_column_text(stmt, 0),
                   *pid = (const char*)sqlite3_column_text(stmt, 1),
                   *pver = (const char*)sqlite3_column_text(stmt, 2),
                   *aver = (const char*)sqlite3_column_text(stmt, 3);
        l += snprintf(&b[l], sizeof(b) - l, DEVICE, sid, pid, pver, aver);
        err = sqlite3_step(stmt);
        if (err == SQLITE_ROW && l < sizeof(b)) {
            l += snprintf(&b[l], sizeof(b) - l, ",");
        }
    }
    sqlite3_finalize(stmt);
    if (l < sizeof(b)) {
        l += snprintf(&b[l], sizeof(b) - l, "}}");
        http_printf_json(ctx->curr_connection, 200, b);
    } else {
        snprintf(b, sizeof(b), "{\"error\":\"Response too large\"}");
        http_printf_json(ctx->curr_connection, 400, b);
    }
}

