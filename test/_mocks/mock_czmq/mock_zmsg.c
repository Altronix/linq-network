// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mock_zmsg.h"

#include "altronix/linq_network.h"
#include <cmocka.h>

typedef struct mock_zmsg_s
{
    struct mock_zmsg_s* next;
    zmsg_t* msg;
} mock_zmsg_s;

static mock_zmsg_s* incoming = NULL;
static mock_zmsg_s* outgoing = NULL;

void
czmq_spy_mesg_reset()
{
    while (incoming) {
        zmsg_t* m = czmq_spy_mesg_pop_incoming();
        zmsg_destroy(&m);
    }
    while (outgoing) {
        zmsg_t* m = czmq_spy_mesg_pop_outgoing();
        zmsg_destroy(&m);
    }
}

static void
czmq_spy_mesg_push(mock_zmsg_s** dir_p, zmsg_t** msg_p)
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
czmq_spy_mesg_pop(mock_zmsg_s** dir_p)
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
czmq_spy_mesg_push_incoming(zmsg_t** msg_p)
{
    czmq_spy_mesg_push(&incoming, msg_p);
}

zmsg_t*
czmq_spy_mesg_pop_incoming()
{
    return czmq_spy_mesg_pop(&incoming);
}

void
czmq_spy_mesg_push_outgoing(zmsg_t** msg_p)
{
    czmq_spy_mesg_push(&outgoing, msg_p);
}

zmsg_t*
czmq_spy_mesg_pop_outgoing()
{
    return czmq_spy_mesg_pop(&outgoing);
}

zmsg_t*
__wrap_zmsg_recv(void* source)
{
    ((void)source);
    // TODO return spy error
    return czmq_spy_mesg_pop_incoming();
}

int
__wrap_zmsg_send(zmsg_t** self_p, void* dest)
{
    ((void)dest);
    czmq_spy_mesg_push_outgoing(self_p);
    // TODO return spy error
    return 0;
}
