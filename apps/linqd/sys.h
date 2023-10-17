// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYS_H
#define SYS_H

#include "netw.h"
#include "stdlib.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>

#define UUID_LEN 16
#define SYS_CONFIG_DIRS(var)                                                   \
    const char* var[4] = { "~/.config/altronix/%s.json",                       \
                           "~/.config/atx/%s.json",                            \
                           "/etc/atx.config.%s.json",                          \
                           NULL }

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
    typedef pid_t sys_pid;

    void optind_set(int val);
    int32_t sys_tick();
    uint32_t sys_unix();
    void sys_uuid(char*);
    sys_file* sys_open(const char*, E_FILE_MODE, E_FILE_BLOCKING);
    uint32_t sys_len(sys_file* f);
    int sys_read_buffer(sys_file*, char*, uint32_t*);
    int sys_read(sys_file*, char**, uint32_t*);
    int sys_write(sys_file*, const char*, uint32_t);
    int sys_vfprintf(sys_file* f, const char* fmt, va_list);
    int sys_fprintf(sys_file* f, const char* fmt, ...);
    void sys_close(sys_file** f_p);
    void sys_make_absolute(const char*, char*, uint32_t*);
    int sys_daemonize(const char*, sys_file**, sys_pid*);
    const char* sys_config_dir(const char* name);
    void sys_msleep(uint32_t);

#ifdef __cplusplus
}
#endif
#endif
