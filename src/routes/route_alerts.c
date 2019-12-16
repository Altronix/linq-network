#include "routes.h"

void
route_alerts(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    ((void)meth);
    ((void)l);
    ((void)body);
    http_printf_json(ctx, 504, "{\"TODO\":\"TODO\"}");
}
