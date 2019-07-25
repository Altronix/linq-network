#ifndef CONTAINERS_H_
#define CONTAINERS_H_

#include "linq_internal.h"

#include "klib/khash.h"
#include "klib/klist.h"

#ifdef __cplusplus
extern "C"
{
#endif

    static inline void __list_free_fn(void* ctx) { ((void)ctx); }
#define FREE_FN(x) __list_free_fn(x->data)

#define LIST_INIT(tag, type, list_free_fn)                                     \
    KLIST_INIT(tag, type*, FREE_FN)                                            \
                                                                               \
    kl_##tag##_t* list_##tag##_create();                                       \
    void list_##tag##_destroy(kl_##tag##_t**);                                 \
    void list_##tag##_push(kl_##tag##_t*, type**);                             \
    type* list_##tag##_pop(kl_##tag##_t*);                                     \
                                                                               \
    typedef kl_##tag##_t list_##tag##_s;                                       \
                                                                               \
    list_##tag##_s* list_##tag##_create() { return kl_init_##tag(); }          \
                                                                               \
    void list_##tag##_destroy(list_##tag##_s** list_p)                         \
    {                                                                          \
        list_##tag##_s* list = *list_p;                                        \
        *list_p = NULL;                                                        \
        type* next = list_##tag##_pop(list);                                   \
        while (next) {                                                         \
            type* deleteme = next;                                             \
            next = list_##tag##_pop(list);                                     \
            list_free_fn(&deleteme);                                           \
        }                                                                      \
        kl_destroy_##tag(list);                                                \
    }                                                                          \
                                                                               \
    void list_##tag##_push(list_##tag##_s* list, type** r_p)                   \
    {                                                                          \
        type* r = *r_p;                                                        \
        *r_p = NULL;                                                           \
        *kl_pushp_##tag(list) = r;                                             \
    }                                                                          \
                                                                               \
    type* list_##tag##_pop(list_##tag##_s* l)                                  \
    {                                                                          \
        type* ret = NULL;                                                      \
        kl_shift_##tag(l, &ret);                                               \
        return ret;                                                            \
    }                                                                          \
                                                                               \
    uint32_t list_##tag##_size(list_##tag##_s* l) { return l->size; }
#ifdef __cplusplus
}
#endif
#endif
