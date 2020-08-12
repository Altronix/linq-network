#ifndef ROUTES_H
#define ROUTES_H

#include "http.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define JERROR_200 "{\"error\":\"Ok\"}"
#define JERROR_400 "{\"error\":\"Bad request\"}"
#define JERROR_404 "{\"error\":\"Not found\"}"
#define JERROR_500 "{\"error\":\"Server error\"}"
#define JERROR_503 "{\"error\":\"Unauthorized\"}"
#define JERROR_504 "{\"error\":\"Server busy\"}"

#define UNSECURE_API "/api/v1/public"
#define UNSECURE_API_LEN (sizeof(UNSECURE_API) - 1)

#define PROTO_ARGS                                                             \
    http_route_context *, HTTP_METHOD meth, uint32_t, const char *

    void route_create_admin(PROTO_ARGS);
    void route_login(PROTO_ARGS);
    void route_users(PROTO_ARGS);
    void route_alerts(PROTO_ARGS);
    void route_devices(PROTO_ARGS);
    void route_proxy(PROTO_ARGS);

#ifdef __cplusplus
}
#endif
#endif
