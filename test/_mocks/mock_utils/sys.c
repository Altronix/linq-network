// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sys.h"
#include <time.h>

static int32_t tick = 0;
static uint32_t __internal_unix = 0;

void
spy_sys_set_tick(int32_t t)
{
    tick = t;
}

void
spy_sys_set_unix(uint32_t t)
{
    __internal_unix = t;
}

void
spy_sys_reset()
{
    tick = 0;
    __internal_unix = 0;
}

int32_t
__wrap_sys_tick()
{
    return tick;
}

uint32_t
__wrap_sys_unix()
{
    return __internal_unix;
}
