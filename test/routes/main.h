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

    helpers_test_context_s* test_init(
        atx_net_callbacks* callbacks,
        void* context,
        const char* user,
        const char* password);
    void test_reset(helpers_test_context_s** test_p);

#ifdef __cplusplus
}
#endif
#endif
