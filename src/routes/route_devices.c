#include "linq_netw_internal.h"
#include "routes.h"

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
    char buff[LINQ_NETW_MAX_RESPONSE_SIZE];
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
    if (count && offset && countl < 6 && offsetl < 6) {
        l = snprintf(buff, sizeof(buff), QUERY, countl, count, offsetl, offset);
    } else {
        l = snprintf(buff, sizeof(buff), QUERY_STATIC);
    }
    err = sqlite3_prepare_v2(db->db, buff, l + 1, &stmt, NULL);
    linq_netw_assert(err == SQLITE_OK);

    // Convert database output to json
    l = snprintf(buff, sizeof(buff), "{\"devices\":{");
    err = sqlite3_step(stmt);
    while (err == SQLITE_ROW && (l < sizeof(buff))) {
        l += snprintf(
            &buff[l],
            sizeof(buff) - l,
            DEVICE,
            sqlite3_column_text(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3));
        err = sqlite3_step(stmt);
        if (err == SQLITE_ROW) l += snprintf(&buff[l], sizeof(buff) - l, ",");
    }
    if (l < sizeof(buff)) {
        l += snprintf(&buff[l], sizeof(buff) - l, "}}");
        sqlite3_finalize(stmt);
        http_printf_json(ctx, 200, buff);
    } else {
        snprintf(buff, sizeof(buff), "{\"error\":\"Response too large\"}");
        http_printf_json(ctx, 400, buff);
    }
}

