// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MOCK_ZPOLL_H_
#define MOCK_ZPOLL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void czmq_spy_poll_reset(void);
    void czmq_spy_poll_set_incoming(uint32_t i);

#ifdef __cplusplus
}
#endif

#endif
