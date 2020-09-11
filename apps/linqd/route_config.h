#ifndef ROUTE_CONFIG_H
#define ROUTE_CONFIG_H

#include "http.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void route_config(
        http_request_s* ctx,
        HTTP_METHOD meth,
        uint32_t l,
        const char* b);

#ifdef __cplusplus
}
#endif
#endif
