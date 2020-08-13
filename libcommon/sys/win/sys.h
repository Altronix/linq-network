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

// clang-format off
#if defined _WIN32
#  if defined LINQ_STATIC
#    define LINQ_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_EXPORT __declspec(dllexport)
#  else
#    define LINQ_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_EXPORT
#endif
// clang-format on

#define linq_network_malloc_fn malloc
#define linq_network_free_fn free
#define linq_network_assert_fn assert

#define linq_network_malloc(x) linq_network_malloc_fn(x)
#define linq_network_free(x) linq_network_free_fn(x)
#define linq_network_assert(x) linq_network_assert_fn(x)

#define memmem sys_memmem

#define UUID_LEN 16

#ifdef __cplusplus
extern "C"
{
#endif

    LINQ_EXPORT extern char* optarg;
    LINQ_EXPORT extern int optind;
    LINQ_EXPORT int optind_get();
    LINQ_EXPORT char* optarg_get();
    LINQ_EXPORT void optind_set(int val);

    LINQ_EXPORT int getopt(int argc, char* const argv[], const char* optstring);
    LINQ_EXPORT int32_t sys_tick();
    LINQ_EXPORT uint32_t sys_unix();
    LINQ_EXPORT void sys_uuid(char* dst);
    LINQ_EXPORT void* sys_memmem(
        const void* haystack,
        size_t haystack_len,
        const void* const needle,
        const size_t needle_len);
#define optind optind_get()
#define optarg optarg_get()

#ifdef __cplusplus
}
#endif
#endif
