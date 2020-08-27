#include "netw.h"
#include "routes.h"

void
route_connect(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t l,
    const char* body)
{
    const jsontok* tok;
    char buffer[128];
    netw_s* netw;
    jsontok toks[32];
    json_value v;
    json_parser p;
    json_init(&p);
    netw = ctx->context;
    int rc = json_parse(&p, body, l, toks, 32);
    if (rc > 0 && (tok = json_delve(body, toks, ".endpoint"))) {
        v = json_tok_value(body, toks);
        snprintf(buffer, sizeof(buffer), "%.*s", v.len, v.p);
        netw_connect(netw, buffer);
        http_printf_json(ctx->curr_connection, 200, "{\"error\":\"Ok\"}");
    } else {
        http_printf_json(ctx->curr_connection, 400, "{\"error\":\"Bad args\"}");
    }
}
