#ifndef MORE_H_
#define MORE_H_

#include "stdint.h"
#include "zmq.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void message_more_set(zmq_msg_t* msg);
    int message_valid(zmq_msg_t* msg);

#ifdef __cplusplus
}
#endif
#endif
