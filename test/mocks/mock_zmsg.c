#include "mock_zmsg.h"

#include "altronix/linq.h"
#include <cmocka.h>

typedef struct mock_zmsg_s
{
    struct mock_zmsg_s* next;
    zmsg_t* msg;
} mock_zmsg_s;

static mock_zmsg_s* incoming = NULL;
static mock_zmsg_s* outgoing = NULL;

static void
czmq_spy_push_mesg(mock_zmsg_s** dir_p, zmsg_t** msg_p)
{
    mock_zmsg_s *seek = *dir_p, *next = malloc(sizeof(mock_zmsg_s));
    assert_non_null(next);
    next->msg = *msg_p;
    next->next = NULL;
    *msg_p = NULL;
    if (!*dir_p) {
        *dir_p = next;
    } else {
        while (seek->next) seek = seek->next;
        seek->next = next;
    }
}

static zmsg_t*
czmq_spy_pop_mesg(mock_zmsg_s** dir_p)
{
    zmsg_t* ret = NULL;
    mock_zmsg_s* next = *dir_p;
    if (!next) return NULL;
    ret = next->msg;
    *dir_p = next->next;
    free(next);
    return ret;
}

void
czmq_spy_push_incoming_mesg(zmsg_t** msg_p)
{
    czmq_spy_push_mesg(&incoming, msg_p);
}

zmsg_t*
czmq_spy_pop_incoming_mesg()
{
    return czmq_spy_pop_mesg(&incoming);
}

void
czmq_spy_push_outgoing_mesg(zmsg_t** msg_p)
{
    czmq_spy_push_mesg(&outgoing, msg_p);
}

zmsg_t*
czmq_spy_pop_outgoing_mesg()
{
    return czmq_spy_pop_mesg(&outgoing);
}


zmsg_t*
__wrap_zmsg_recv(void* source)
{
    ((void)source);
    return czmq_spy_pop_incoming_mesg();
}

int
__wrap_zmsg_send(zmsg_t** self_p, void* dest)
{
    ((void)self_p);
    ((void)dest);
    return 0;
}
