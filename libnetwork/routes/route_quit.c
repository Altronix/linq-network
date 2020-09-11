#include "netw.h"
#include "routes.h"

void
route_quit(http_request_s* r, HTTP_METHOD meth, uint32_t jlen, const char* body)
{
    netw_s* netw = http_request_context(r);
    netw_shutdown(netw);
    http_printf_json(
        r->connection,
        http_error_code(0),
        "{\"error\":\"%s\"}",
        http_error_message(0));
}
