// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYS_H
#define SYS_H

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    void spy_sys_set_tick(int64_t t);
    void spy_sys_reset();

#ifdef __cplusplus
}
#endif
#endif
