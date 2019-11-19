#ifndef SYS_H
#define SYS_H

#include <stdint.h>

#define linq_netw_malloc_fn malloc
#define linq_netw_free_fn free
#define linq_netw_assert_fn assert

#define linq_netw_malloc(x) linq_netw_malloc_fn(x)
#define linq_netw_free(x) linq_netw_free_fn(x)
#define linq_netw_assert(x) linq_netw_assert_fn(x)

#ifdef __cplusplus
extern "C"
{
#endif

    int64_t sys_tick();

#ifdef __cplusplus
}
#endif
#endif
