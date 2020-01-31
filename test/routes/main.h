#ifndef MAIN_H
#define MAIN_H

#include "helpers.h"
#include "sys.h"

#define USER "unsafe_user"
#define PASS "unsafe_pass"

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
