#ifndef ROUTES_H
#define ROUTES_H

#include "http.h"
#include "linq_netw_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void route_alerts(http_route_context*, HTTP_METHOD, uint32_t, const char*);
    void route_devices(http_route_context*, HTTP_METHOD, uint32_t, const char*);
    void route_proxy(http_route_context*, HTTP_METHOD, uint32_t, const char*);

#ifdef __cplusplus
}
#endif
#endif
