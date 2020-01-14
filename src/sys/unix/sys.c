// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "sys.h"
#include <time.h>

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
