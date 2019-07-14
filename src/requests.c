#include "requests.h"
#include "klib/klist.h"

void request_destroy(request_s** r_p);

static inline void
request_free_fn(request_s* r)
{
    ((void)r);
}

#define REQUEST_FREE_FN(x) request_free_fn(x->data)

KLIST_INIT(requests, request_s*, REQUEST_FREE_FN);

typedef struct requests_s
{
    klist_t(requests) * list;
} requests_s;

typedef struct kl_requests_t request_lists_s;
requests_s*
requests_create()
{
    requests_s* requests = linq_malloc(sizeof(requests_s));
    if (requests) {
        memset(requests, 0, sizeof(requests_s));
        requests->list = kl_init(requests);
    }
    return requests;
}

void
requests_destroy(requests_s** list_p)
{
    request_s* next;
    requests_s* requests = *list_p;
    *list_p = NULL;
    next = requests_pop(requests);
    while (next) {
        request_s* deleteme = next;
        next = requests_pop(requests);
        request_destroy(&deleteme);
    }
    kl_destroy(requests, requests->list);
    linq_free(requests);
}

void
requests_push(requests_s* list, request_s** r_p)
{
    request_s* r = *r_p;
    *r_p = NULL;
    *kl_pushp(requests, list->list) = r;
}

request_s*
requests_pop(requests_s* list)
{
    request_s* r = NULL;
    kl_shift(requests, list->list, &r);
    return r;
}

uint32_t
requests_size(requests_s* requests)
{
    return requests->list->size;
}
