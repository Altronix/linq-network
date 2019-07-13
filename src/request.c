#include "czmq.h"

#include "klib/klist.h"
#include "request.h"

typedef struct request_s
{
    linq_request_complete_fn on_complete;
    zframe_t* frames[FRAME_REQ_DATA_IDX + 1];
} request_s;

void request_destroy(request_s** r_p);

static inline void
request_free_fn(request_s* r)
{
    ((void)r);
    // if (r) request_destroy(&r);
}

#define REQUEST_FREE_FN(x) request_free_fn(x->data)

KLIST_INIT(requests, request_s*, REQUEST_FREE_FN);

request_s*
request_create(
    const char* serial,
    const char* path,
    const char* json,
    linq_request_complete_fn on_complete)
{
    return request_create_mem(
        serial,
        strlen(serial),
        path,
        strlen(path),
        json,
        strlen(json),
        on_complete);
}

request_s*
request_create_mem(
    const char* s,
    uint32_t slen,
    const char* p,
    uint32_t plen,
    const char* d,
    uint32_t dlen,
    linq_request_complete_fn fn)
{
    request_s* r = linq_malloc(sizeof(request_s));
    if (r) {
        memset(r, 0, sizeof(request_s));
        r->on_complete = fn;
        r->frames[FRAME_VER_IDX] = zframe_new("\0", 1);
        r->frames[FRAME_TYP_IDX] = zframe_new("\1", 1);
        r->frames[FRAME_SID_IDX] = zframe_new(s, slen);
        r->frames[FRAME_REQ_PATH_IDX] = zframe_new(p, plen);
        r->frames[FRAME_REQ_DATA_IDX] = d && dlen ? zframe_new(d, dlen) : NULL;
        if (!(r->frames[FRAME_VER_IDX] && r->frames[FRAME_TYP_IDX] &&
              r->frames[FRAME_SID_IDX] && r->frames[FRAME_REQ_PATH_IDX] &&
              r->frames[FRAME_REQ_DATA_IDX])) {
            request_destroy(&r);
        }
    }
    return r;
}

void
request_destroy(request_s** r_p)
{
    request_s* r = *r_p;
    *r_p = NULL;
    for (uint32_t i = 0; i < (sizeof(r->frames) / sizeof(zframe_t*)); i++) {
        if (r->frames[i]) zframe_destroy(&r->frames[i]);
    }
    linq_free(r);
}

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
