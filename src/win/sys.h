#ifndef SYS_H
#define SYS_H

#include <stdint.h>

#define linq_io_malloc_fn malloc
#define linq_io_free_fn free
#define linq_io_assert_fn assert

#define linq_io_malloc(x) linq_io_malloc_fn(x)
#define linq_io_free(x) linq_io_free_fn(x)
#define linq_io_assert(x) linq_io_assert_fn(x)

#ifdef __cplusplus
extern "C"
{
#endif

    int64_t sys_tick();

#ifdef __cplusplus
}
#endif
#endif
