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
    "\"time\":\"%s\","                                                         \
    "\"mesg\":\"%s\""                                                          \
    "}"

void
route_alerts(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t _l,
    const char* _body)
{
    ((void)meth);
    ((void)_l);
    ((void)_body);
    http_printf_json(ctx, 504, "{\"TODO\":\"TODO\"}");
}
