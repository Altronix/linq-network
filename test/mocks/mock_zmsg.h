#ifndef MOCK_ZMSG_H_
#define MOCK_ZMSG_H_

#include <czmq.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void czmq_spy_mesg_push_incoming(zmsg_t** msg_p);
    void czmq_spy_mesg_push_outgoing(zmsg_t** msg_p);
    void czmq_spy_mesg_reset(void);
    zmsg_t* czmq_spy_mesg_pop_outgoing();
    zmsg_t* czmq_spy_mesg_pop_incoming();

#ifdef __cplusplus
}
#endif
#endif
