// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "atx_net_internal.h"

extern volatile int zsys_interrupted;

bool
sys_running()
{
    return !zsys_interrupted;
}