// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mock_zpoll.h"
#include "zmq.h"

static uint32_t incoming = 0;

void
czmq_spy_poll_set_incoming(uint32_t val)
{
    incoming = val;
}

void
czmq_spy_poll_reset()
{
    incoming = 0;
}

int
__wrap_zmq_poll(zmq_pollitem_t* items_, int nitems_, long timeout_)
{
    ((void)timeout_);
    for (int i = 0; i < nitems_ && i < 32; i++) {
        if (incoming & (0x01 << i)) {
            items_[i].revents |= ZMQ_POLLIN;
        } else {
            items_[i].revents &= (~(ZMQ_POLLIN));
        }
    }
    return 0;
}
