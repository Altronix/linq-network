#ifndef MOCK_ZPOLL_H_
#define MOCK_ZPOLL_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void czmq_spy_poll_reset(void);
    void czmq_spy_poll_push_incoming(bool i);

#ifdef __cplusplus
}
#endif

#endif
