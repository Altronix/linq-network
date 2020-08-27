#include "netw.h"
#include "routes.h"

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
    device_s d;
    database_s* db = netw_database(ctx->context);

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
    l = snprintf(b, sizeof(b), "{\"devices\":{");
    err = database_device_open(db, &d, countl, offsetl);
    if (!err) {
        while (err != DATABASE_DONE && l < sizeof(b)) {
            l += snprintf(
                &b[l],
                sizeof(b) - l,
                DEVICE,
                d.id,
                d.product,
                d.prj_version,
                d.atx_version);
            err = database_device_next(&d);
            if (err != DATABASE_DONE && l < sizeof(b)) {
                l += snprintf(&b[l], sizeof(b) - l, ",");
            }
        }
        database_device_close(&d);
    }
    if (l < sizeof(b)) {
        l += snprintf(&b[l], sizeof(b) - l, "}}");
        http_printf_json(ctx->curr_connection, 200, b);
    } else {
        snprintf(b, sizeof(b), "{\"error\":\"Response too large\"}");
        http_printf_json(ctx->curr_connection, 400, b);
    }
}

