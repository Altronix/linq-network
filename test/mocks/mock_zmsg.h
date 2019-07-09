#ifndef MOCK_ZMSG_H_
#define MOCK_ZMSG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <czmq.h>

    void czmq_spy_push_incoming_mesg(zmsg_t** msg_p);
    void czmq_spy_push_outgoing_mesg(zmsg_t** msg_p);
    zmsg_t* czmq_spy_pop_outgoing_mesg();

#ifdef __cplusplus
}
#endif
#endif
