#include "netw.h"
#include "routes.h"

void
route_quit(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t jlen,
    const char* body)
{
    netw_s* netw = ctx->context;
    netw_shutdown(netw);
    http_printf_json(
        ctx->curr_connection,
        http_error_code(0),
        "{\"error\":\"%s\"}",
        http_error_message(0));
}
