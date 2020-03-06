// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"

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
    uuid_set(dst, (uint8_t*)uuid);
}
