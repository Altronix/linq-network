// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"
#include "log.h"

#include <Rpc.h>

#define _WINSOCKAPI_
#include <windows.h>

#undef optind
#undef optarg

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
    return 0;
}

uint32_t
sys_unix()
{
    return 0;
}

void
sys_uuid(char* dst)
{
    UUID uuid;
    linq_network_assert(sizeof(uuid) == UUID_LEN);
    UuidCreate(&uuid);
    uuid_set(dst, (uint8_t*)&uuid);
}

int
sys_daemonize(const char* log, sys_file** f, sys_pid* pid)
{
    log_error("(SYS) daemon mode not supported on windows atm");
    return -1;
}

