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

#define PROTO_ARGS                                                             \
    http_route_context *, HTTP_METHOD meth, uint32_t, const char *

    void create_admin(PROTO_ARGS);
    void login(PROTO_ARGS);
    void alerts(PROTO_ARGS);
    void devices(PROTO_ARGS);
    void proxy(PROTO_ARGS);
    void scan(PROTO_ARGS);
    void quit(PROTO_ARGS);

#ifdef __cplusplus
}
#endif
#endif
