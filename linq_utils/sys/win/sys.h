// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYS_H
#define SYS_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define linq_network_malloc_fn malloc
#define linq_network_free_fn free
#define linq_network_assert_fn assert

#define linq_network_malloc(x) linq_network_malloc_fn(x)
#define linq_network_free(x) linq_network_free_fn(x)
#define linq_network_assert(x) linq_network_assert_fn(x)

#define UUID_LEN 16

#ifdef __cplusplus
extern "C"
{
#endif

    extern char* optarg;
    extern int optind;

    int getopt(int argc, char* const argv[], const char* optstring);
    int32_t sys_tick();
    uint32_t sys_unix();
    void sys_uuid(char* dst);

#ifdef __cplusplus
}
#endif
#endif
