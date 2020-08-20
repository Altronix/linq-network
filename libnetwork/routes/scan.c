#include "netw.h"
#include "routes.h"

void
scan(http_route_context* ctx, HTTP_METHOD meth, uint32_t jlen, const char* body)
{
    int rc;
    netw_s* netw = ctx->context;
    rc = netw_scan(netw);
    http_printf_json(ctx->curr_connection, 200, "{\"count\":%d}", rc);
}
