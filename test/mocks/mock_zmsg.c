#include "mock_zmsg.h"

typedef struct mock_zmsg_s
{
    struct mock_zmsg_s* next;
    zmsg_t* msg;
} mock_zmsg_s;

static mock_zmsg_s* incoming = NULL;

void
czmq_spy_push_incoming_mesg(zmsg_t** msg_p)
{
    mock_zmsg_s *seek = incoming, *next = malloc(sizeof(mock_zmsg_s));
    assert(next);
    next->msg = *msg_p;
    next->next = NULL;
    *msg_p = NULL;
    if (!incoming) {
        incoming = next;
    } else {
        while (seek->next) seek = seek->next;
        seek->next = next;
    }
}

zmsg_t*
__wrap_zmsg_recv(void* source)
{
    ((void)source);
    zmsg_t* ret = NULL;
    mock_zmsg_s* next = incoming;
    if (!next) return NULL;
    ret = next->msg;
    incoming = next->next;
    free(next);
    return ret;
}

int
__wrap_zmsg_send(zmsg_t** self_p, void* dest)
{
    ((void)self_p);
    ((void)dest);
    return 0;
}
