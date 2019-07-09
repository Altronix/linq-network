#ifndef MOCK_ZMSG_H_
#define MOCK_ZMSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <czmq.h>

void czmq_spy_push_incoming_mesg(zmsg_t** msg_p);

#ifdef __cplusplus
}
#endif
#endif
