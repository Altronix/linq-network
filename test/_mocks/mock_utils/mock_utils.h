// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MOCK_UTILS_H
#define MOCK_UTILS_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define linq_network_malloc_fn malloc
#define linq_network_free_fn free
#define linq_network_assert_fn assert

#define linq_network_malloc(x) linq_network_malloc_fn(x)
#define linq_network_free(x) linq_network_free_fn(x)
#define linq_network_assert(x) linq_network_assert_fn(x)

#ifdef __cplusplus
extern "C"
{
#endif

    void spy_sys_set_tick(int32_t t);
    void spy_sys_set_unix(uint32_t t);
    void spy_sys_reset();

#ifdef __cplusplus
}
#endif
#endif
