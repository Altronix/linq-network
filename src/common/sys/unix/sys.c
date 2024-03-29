// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"
#include "common/log.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

static int
set_blocking(sys_file* fd, bool block)
{
    int flags;
    flags = fcntl(fileno(fd), F_GETFL, 0);
    if (flags == -1) return -EFAULT;
    return fcntl(
        fileno(fd), F_SETFL, block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
}

LINQ_EXPORT void
optind_set(int val)
{
    optind = val;
}

static void
uuid_set(char* dst, const uint8_t* src)
{
    char hex_char[] = "0123456789ABCDEF";
    int n;
    for (n = 0; n < UUID_LEN; n++) {
        uint val = (src)[n];
        dst[n * 2 + 0] = hex_char[val >> 4];
        dst[n * 2 + 1] = hex_char[val & 15];
    }
    dst[UUID_LEN * 2] = 0;
}

int32_t
sys_tick()
{
    struct timespec ts;
    static int64_t tick = 0;
    int64_t now = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    now = (int64_t)((int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000);
    if (!tick) tick = now;
    return now - tick;
}

uint32_t
sys_unix()
{
    return (uint32_t)time(NULL);
}

void
sys_uuid(char* dst)
{
    uuid_t uuid;
    linq_network_assert(sizeof(uuid) == UUID_LEN);
    uuid_generate(uuid);
    uuid_set(dst, (uint8_t*)uuid);
}

sys_file*
sys_open(const char* path, E_FILE_MODE mode, E_FILE_BLOCKING blocking)
{
    static const char* modes[] = { "r", "w", "a", "r+", "w+", "a+" };
    FILE* f = fopen(path, modes[mode]);
    if (f) set_blocking(f, blocking ? true : false);
    return f;
}

uint32_t
sys_len(sys_file* f)
{
    int n, rc;
    rc = ioctl(fileno(f), FIONREAD, &n);
    return rc == 0 ? n : 0;
}

int
sys_read_buffer(sys_file* f, char* buffer, uint32_t* sz)
{
    return sys_read(f, &buffer, sz);
}

int
sys_read(sys_file* f, char** data_p, uint32_t* len)
{
    int err = 0, n = 0;
    ioctl(fileno(f), FIONREAD, &n);
    if (n > 0) {
        if (!(*data_p)) (*data_p = malloc(n), *len = n);
        assert(*data_p);
        err = fread(*data_p, 1, *len < n ? *len : n, f);
        *len = n;
        if (err == -1 && errno == EAGAIN) err = 0;
    }
    return err;
}

int
sys_write(sys_file* f, const char* data, uint32_t len)
{
    return fwrite(data, 1, len, f);
}

int
sys_vfprintf(sys_file* f, const char* fmt, va_list list)
{
    return vfprintf(f, fmt, list);
}

int
sys_fprintf(sys_file* f, const char* fmt, ...)
{
    int err, len;
    va_list list;

    va_start(list, fmt);
    err = vfprintf(f, fmt, list);
    va_end(list);
    return err;
}

void
sys_close(sys_file** f_p)
{
    sys_file* f = *f_p;
    fclose(f);
    *f_p = NULL;
}

void
sys_make_absolute(const char* path, char* buffer, uint32_t* l)
{
    uint32_t c;
    if (*path == '/' || *path == '\\') {
        // Already absolute
        *l = snprintf(buffer, *l, "%s", path);
    } else {
        getcwd(buffer, *l);
        c = strlen(buffer);
        if (c < *l - c) buffer[c++] = '/';
        *l = snprintf(&buffer[c], *l - c, "%s", path);
    }
}

int
sys_daemonize(const char* log, sys_file** f, sys_pid* pid)
{
    // www.netzmafia.de/skripten/unix/linux-daemon-howto.html
    // Fork parent
    // Exit if parent
    // Erase user
    // create log
    // create new sid
    // change cwd
    char buff[128];
    uint32_t l = sizeof(buff);
    int err = fork();
    if (err < 0) exit(EXIT_FAILURE);
    if (err > 0) exit(EXIT_SUCCESS);
    umask(0);
    if (log) *f = sys_open(log, FILE_MODE_READ_APPEND_CREATE, FILE_BLOCKING);
    *pid = setsid();
    if (*pid < 0) exit(EXIT_FAILURE);
    err = chdir("/");
    if (err) exit(EXIT_FAILURE);
    if (*f) {
        log_set_fd(*f);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    return err;
}

void
sys_msleep(uint32_t val)
{
    usleep(val * 1000);
}
