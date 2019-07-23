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
