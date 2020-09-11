#include "netw.h"
#include "routes.h"

void
route_scan(http_request_s* r, HTTP_METHOD meth, uint32_t jlen, const char* body)
{
    int rc;
    netw_s* netw = http_request_context(r);
    rc = netw_scan(netw);
    http_printf_json(r->connection, 200, "{\"count\":%d}", rc);
}
