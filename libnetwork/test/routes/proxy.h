#ifndef PROXY_H
#define PROXY_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void test_route_proxy_get(void** context_p);
    void test_route_proxy_post(void** context_p);
    void test_route_proxy_404(void** context_p);
    void test_route_proxy_400_too_short(void** context_p);

#ifdef __cplusplus
}
#endif
#endif
