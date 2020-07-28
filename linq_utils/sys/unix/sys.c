// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

LINQ_UTILS_EXPORT void
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
