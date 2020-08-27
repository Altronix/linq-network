#include "netw.h"
#include "routes.h"

#define ALERT                                                                  \
    "{"                                                                        \
    "\"alert_id\":\"%s\","                                                     \
    "\"device_id\":\"%s\","                                                    \
    "\"who\":\"%s\","                                                          \
    "\"what\":\"%s\","                                                         \
    "\"site_id\":\"%s\","                                                      \
    "\"when\":%d,"                                                             \
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
    database_s* db = netw_database(ctx->context);
    alert_s a;

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
        snprintf(b, sizeof(b), "%.*s", countl, count);
        countl = atoi(b);
        snprintf(b, sizeof(b), "%.*s", offsetl, offset);
        offsetl = atoi(b);
    }

    // Convert database output to json
    l = snprintf(b, sizeof(b), "{\"alerts\":[");
    err = database_alert_open(db, &a, countl, offsetl);
    if (!err) {
        while (err != DATABASE_DONE && l < sizeof(b)) {
            l += snprintf(
                &b[l],
                sizeof(b) - l,
                ALERT,
                a.id,
                a.device,
                a.who,
                a.what,
                a.site,
                a.time,
                a.mesg);
            // err = sqlite3_step(stmt);
            err = database_alert_next(&a);
            if (err != DATABASE_DONE && l < sizeof(b)) {
                l += snprintf(&b[l], sizeof(b) - l, ",");
            }
        }
        database_alert_close(&a);
    }
    if (l < sizeof(b)) {
        l += snprintf(&b[l], sizeof(b) - l, "]}");
        http_printf_json(ctx->curr_connection, 200, b);
    } else {
        snprintf(b, sizeof(b), "{\"error\":\"Response too large\"}");
        http_printf_json(ctx->curr_connection, 400, b);
    }
}
