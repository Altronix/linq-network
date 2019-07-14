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

typedef struct request_list_s
{
    klist_t(requests) * list;
} request_list_s;

typedef struct kl_requests_t request_lists_s;
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
    request_s* next;
    request_list_s* requests = *list_p;
    *list_p = NULL;
    next = request_list_pop(requests);
    while (next) {
        request_s* deleteme = next;
        next = request_list_pop(requests);
        request_destroy(&deleteme);
    }
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

request_s*
request_list_pop(request_list_s* list)
{
    request_s* r = NULL;
    kl_shift(requests, list->list, &r);
    return r;
}

uint32_t
request_list_size(request_list_s* requests)
{
    return requests->list->size;
}
