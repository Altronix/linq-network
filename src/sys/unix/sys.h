// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYS_H
#define SYS_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define atx_net_malloc_fn malloc
#define atx_net_free_fn free
#define atx_net_assert_fn assert

#define atx_net_malloc(x) atx_net_malloc_fn(x)
#define atx_net_free(x) atx_net_free_fn(x)
#define atx_net_assert(x) atx_net_assert_fn(x)

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t sys_tick();
    uint32_t sys_unix();

#ifdef __cplusplus
}
#endif
#endif
