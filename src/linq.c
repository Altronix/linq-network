#include "linq_internal.h"

typedef struct linq
{
    void* context;
    zsock_t* sock;
    linq_callbacks* callbacks;
} linq;

linq*
linq_create(linq_callbacks* cb, void* context)
{
    linq* l = linq_malloc(sizeof(linq));
    if (l) {
        memset(l, 0, sizeof(linq));
        l->callbacks = cb;
        l->context = context;
    }
    return l;
}

void
linq_destroy(linq** linq_p)
{
    linq* l = *linq_p;
    *linq_p = NULL;
    linq_free(l);
}

e_linq_error
linq_listen(linq* l, const char* ep)
{
    if (l->sock) return e_linq_bad_args;
    l->sock = zsock_new_router(ep);
    return l->sock ? e_linq_ok : e_linq_bad_args;
}

e_linq_error
linq_poll(linq* l)
{
    int err;
    zmq_pollitem_t item = { zsock_resolve(l->sock), 0, ZMQ_POLLIN, 0 };
    err = zmq_poll(&item, 1, 1000);
    if (err < 0) return err;
    // if (item.revents && ZMQ_POLLIN) { err = linq_recv(l); }
    return err;
}
