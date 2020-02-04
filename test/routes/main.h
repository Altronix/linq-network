#ifndef MAIN_H
#define MAIN_H

#include "helpers.h"
#include "http_auth_unsafe.h"
#include "sys.h"

#define USER UNSAFE_USER
#define PASS UNSAFE_PASS

#ifdef __cplusplus
extern "C"
{
#endif

    helpers_test_context_s* test_init(helpers_test_config_s* config);
    void test_reset(helpers_test_context_s** test_p);

#ifdef __cplusplus
}
#endif
#endif
