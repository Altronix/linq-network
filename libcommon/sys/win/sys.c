// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"
#include "log.h"
#include "time.h"
#include <sys/timeb.h>

#include <Rpc.h>

#define _WINSOCKAPI_
#include <windows.h>

#undef optind
#undef optarg

static int
set_blocking(sys_file* fd, bool block)
{}

char* optarg = NULL;
int optind = 1;

int
optind_get()
{
    return optind;
}

void
optind_set(int val)
{
    optind = val;
}

char*
optarg_get()
{
    return optarg;
}

int
getopt(int argc, char* const argv[], const char* optstring)
{
    if ((optind >= argc) || (argv[optind][0] != '-') ||
        (argv[optind][0] == 0)) {
        return -1;
    }

    int opt = argv[optind][1];
    const char* p = strchr(optstring, opt);

    if (p == NULL) { return '?'; }
    if (p[1] == ':') {
        optind++;
        if (optind >= argc) { return '?'; }
        optarg = argv[optind];
        optind++;
    }
    return opt;
}

void*
sys_memmem(
    const void* haystack,
    size_t haystack_len,
    const void* const needle,
    const size_t needle_len)
{
    if (haystack == NULL) return NULL; // or assert(haystack != NULL);
    if (haystack_len == 0) return NULL;
    if (needle == NULL) return NULL; // or assert(needle != NULL);
    if (needle_len == 0) return NULL;

    for (const char* h = haystack; haystack_len >= needle_len;
         ++h, --haystack_len) {
        if (!memcmp(h, needle, needle_len)) { return h; }
    }
    return NULL;
}

static void
uuid_set(char* dst, const uint8_t* src)
{
    char hex_char[] = "0123456789ABCDEF";
    int n;
    for (n = 0; n < UUID_LEN; n++) {
        uint8_t val = (src)[n];
        dst[n * 2 + 0] = hex_char[val >> 4];
        dst[n * 2 + 1] = hex_char[val & 15];
    }
    dst[UUID_LEN * 2] = 0;
}

int32_t
sys_tick()
{
    static int64_t start_tick = 0;
    int64_t now;
    struct __timeb64 t;
    _ftime64_s(&t);
    now = t.time * 1000 + t.millitm;
    if (!start_tick) start_tick = now;
    return now - start_tick;
}

uint32_t
sys_unix()
{
    struct __timeb64 t;
    _ftime64_s(&t);
    return t.time;
}

void
sys_uuid(char* dst)
{
    UUID uuid;
    linq_network_assert(sizeof(uuid) == UUID_LEN);
    UuidCreate(&uuid);
    uuid_set(dst, (uint8_t*)&uuid);
}

sys_file*
sys_open(const char* path, E_FILE_MODE mode, E_FILE_BLOCKING blocking)
{
    return NULL;
}

uint32_t
sys_len(sys_file* f)
{
    return 0;
}

int
sys_read_buffer(sys_file* f, char* buffer, uint32_t* sz)
{
    return -1;
}

int
sys_read(sys_file* f, char** data_p, uint32_t* len)
{
    return -1;
}

int
sys_write(sys_file* f, const char* data, uint32_t len)
{
    return -1;
}

int
sys_vfprintf(sys_file* f, const char* fmt, va_list list)
{
    return -1;
}

int
sys_fprintf(sys_file* f, const char* fmt, ...)
{
    return -1;
}

void
sys_close(sys_file** f_p)
{}

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
    log_error("SYS", "daemon mode not supported on windows atm");
    return -1;
}

