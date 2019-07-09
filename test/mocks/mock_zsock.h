#ifndef MOCK_ZSOCK_H_
#define MOCK_ZSOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <czmq.h>

int czmq_spy_push_incoming_mesg(zmsg_t** msg_p);

#ifdef __cplusplus
}
#endif
#endif
