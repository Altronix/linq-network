#ifndef ROUTES_H
#define ROUTES_H

#if defined BUILD_LINQD
#include "http.h"
#endif

#ifndef LINQ_NETW_MAX_RESPONSE_SIZE
#define LINQ_NETW_MAX_RESPONSE_SIZE 8096
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PROTO_ARGS http_request_s *, HTTP_METHOD meth, uint32_t, const char *

    void route_create_admin(PROTO_ARGS);
    void route_login(PROTO_ARGS);
    void route_alerts(PROTO_ARGS);
    void route_devices(PROTO_ARGS);
    void route_proxy(PROTO_ARGS);
    void route_connect(PROTO_ARGS);
    void route_scan(PROTO_ARGS);
    void route_quit(PROTO_ARGS);

#ifdef __cplusplus
}
#endif
#endif
