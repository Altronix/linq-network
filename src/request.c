#include "czmq.h"

#include "klib/klist.h"
#include "request.h"

typedef struct request_s
{
    linq_request_complete_fn on_complete;
    int n;
    zframe_t* frames[];
} request_s;

void request_destroy(request_s** r_p);

static inline void
request_free_fn(request_s* r)
{
    request_destroy(&r);
}

#define REQUEST_FREE_FN(x) request_free_fn(x->data)

KLIST_INIT(requests, request_s*, REQUEST_FREE_FN);

request_s*
request_create()
{
    return NULL;
}

void
request_destroy(request_s** r_p)
{
    ((void)r_p);
}

typedef struct request_list_s
{
    klist_t(requests) * list;
} request_list_s;

request_list_s*
request_list_create()
{
    request_list_s* requests = linq_malloc(sizeof(request_list_s));
    if (requests) {
        memset(requests, 0, sizeof(request_list_s));
        requests->list = kl_init(requests);
    }
    return requests;
}

void
request_list_destroy(request_list_s** list_p)
{
    request_list_s* requests = *list_p;
    *list_p = NULL;
    kl_destroy(requests, requests->list);
    linq_free(requests);
}

void
request_list_push(request_list_s* list, request_s** r_p)
{
    request_s* r = *r_p;
    *r_p = NULL;
    *kl_pushp(requests, list->list) = r;
}
