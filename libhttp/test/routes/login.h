#ifndef LOGIN_H
#define LOGIN_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void test_route_login_ok(void** context_p);
    void test_route_login_bad_pass(void** context_p);

#ifdef __cplusplus
}
#endif
#endif
