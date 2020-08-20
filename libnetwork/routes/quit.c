#include "netw.h"
#include "routes.h"

void
quit(http_route_context* ctx, HTTP_METHOD meth, uint32_t jlen, const char* body)
{
    netw_s* netw = ctx->context;
    netw_shutdown(netw);
    http_printf_json(
        ctx->curr_connection,
        200,
        "{\"error\":\"%s\"}",
        http_error_message(200));
}
