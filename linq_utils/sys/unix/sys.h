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
#include <unistd.h>

// clang-format off
#if defined _WIN32
#  if defined LINQ_UTILS_STATIC
#    define LINQ_UTILS_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_UTILS_EXPORT __declspec(dllexport)
#  else
#    define LINQ_UTILS_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_UTILS_EXPORT
#endif
// clang-format on

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

    typedef enum E_FILE_MODE
    {
        FILE_MODE_READ = 0,               // READ ONLY (MUST EXIST)
        FILE_MODE_WRITE = 1,              // WRITE (DESTROY IF EXISTS)
        FILE_MODE_APPEND = 2,             // APPEND (CREATE IF NOT EXISTS)
        FILE_MODE_READ_WRITE = 3,         // READ/WRITE (MUST EXIST)
        FILE_MODE_READ_WRITE_CREATE = 4,  // READ/WRITE (DESTROY IF EXISTS)
        FILE_MODE_READ_APPEND_CREATE = 5, // READ/APPEND (CREATE IF NOT EXISTS)
    } E_FILE_MODE;

    typedef FILE sys_file;
    typedef pid_t sys_pid;

    LINQ_UTILS_EXPORT void optind_set(int val);
    LINQ_UTILS_EXPORT int32_t sys_tick();
    LINQ_UTILS_EXPORT uint32_t sys_unix();
    LINQ_UTILS_EXPORT void sys_uuid(char*);
    LINQ_UTILS_EXPORT sys_file* sys_open(const char* path, E_FILE_MODE mode);
    LINQ_UTILS_EXPORT int sys_read_buffer(sys_file*, char*, uint32_t*);
    LINQ_UTILS_EXPORT int sys_read(sys_file*, char**, uint32_t*);
    LINQ_UTILS_EXPORT int sys_write(sys_file*, const char*, uint32_t);
    LINQ_UTILS_EXPORT void sys_close(sys_file** f_p);
    LINQ_UTILS_EXPORT void sys_make_absolute(const char*, char*, uint32_t*);
    LINQ_UTILS_EXPORT int sys_daemonize(const char*, sys_file**, sys_pid*);
    LINQ_UTILS_EXPORT void sys_msleep(uint32_t);

#ifdef __cplusplus
}
#endif
#endif
