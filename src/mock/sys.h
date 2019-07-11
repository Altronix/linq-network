#ifndef SYS_H
#define SYS_H

#include <stdint.h>

#define linq_malloc_fn malloc
#define linq_free_fn free
#define linq_assert_fn assert

#define linq_malloc(x) linq_malloc_fn(x)
#define linq_free(x) linq_free_fn(x)
#define linq_assert(x) linq_assert_fn(x)

#ifdef __cplusplus
extern "C"
{
#endif

    int64_t sys_tick();
    void spy_sys_set_tick(int64_t t);
    void spy_sys_reset();

#ifdef __cplusplus
}
#endif
#endif
