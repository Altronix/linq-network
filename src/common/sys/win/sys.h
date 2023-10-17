// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYS_H
#define SYS_H

#include "netw.h"

#define memmem sys_memmem

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

    typedef enum E_FILE_BLOCKING
    {
        FILE_NON_BLOCKING = 0,
        FILE_BLOCKING = 1
    } E_FILE_BLOCKING;

    typedef FILE sys_file;
    typedef int sys_pid;

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
    LINQ_EXPORT sys_file* sys_open(const char*, E_FILE_MODE, E_FILE_BLOCKING);
    LINQ_EXPORT uint32_t sys_len(sys_file* f);
    LINQ_EXPORT int sys_read_buffer(sys_file*, char*, uint32_t*);
    LINQ_EXPORT int sys_read(sys_file*, char**, uint32_t*);
    LINQ_EXPORT int sys_write(sys_file*, const char*, uint32_t);
    LINQ_EXPORT int sys_vfprintf(sys_file* f, const char* fmt, va_list);
    LINQ_EXPORT int sys_fprintf(sys_file* f, const char* fmt, ...);
    LINQ_EXPORT void sys_close(sys_file** f_p);
    LINQ_EXPORT void sys_make_absolute(const char*, char*, uint32_t*);

    LINQ_EXPORT int sys_daemonize(const char* log, sys_file** f, sys_pid* pid);
#define optind optind_get()
#define optarg optarg_get()

#ifdef __cplusplus
}
#endif
#endif
