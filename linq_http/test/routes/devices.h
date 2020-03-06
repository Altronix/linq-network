#ifndef DEVICES_H
#define DEVICES_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void test_route_devices(void** context_p);
    void test_route_devices_response_too_large(void** context_p);
    void test_route_devices_response_get_only(void** context_p);
    void test_route_devices_response_empty(void** context_p);

#ifdef __cplusplus
}
#endif
#endif
