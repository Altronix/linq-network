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
    typedef kl1_##tag tag##_item_s;                                            \
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
    type* list_##tag##_front(list_##tag##_s* list)                             \
    {                                                                          \
        return list->head->data;                                               \
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

#define MAP_INIT(tag, type, map_free_fn)                                       \
    KHASH_MAP_INIT_STR(tag, type*)                                             \
                                                                               \
    typedef struct kh_##tag##_s map_##tag##_s;                                 \
    typedef void (*map_##tag##_foreach_fn)(void*, type**);                     \
                                                                               \
    map_##tag##_s* map_##tag##_create() { return kh_init_##tag(); }            \
                                                                               \
    void map_##tag##_destroy(map_##tag##_s** map_p)                            \
    {                                                                          \
        map_##tag##_s* hash = *map_p;                                          \
        *map_p = NULL;                                                         \
        for (khint_t k = kh_begin(hash); k != kh_end(hash); ++k) {             \
            if (kh_exist(hash, k)) {                                           \
                type* d = kh_val(hash, k);                                     \
                map_free_fn(&d);                                               \
            }                                                                  \
        }                                                                      \
        kh_destroy_##tag(hash);                                                \
    }                                                                          \
                                                                               \
    type** map_##tag##_add(                                                    \
        map_##tag##_s* nodes, const char* key, type** node_p)                  \
    {                                                                          \
        int ret = 0;                                                           \
        type* node = *node_p;                                                  \
        *node_p = NULL;                                                        \
        khiter_t k = kh_put_##tag(nodes, key, &ret);                           \
        linq_assert(ret == 1);                                                 \
        kh_val(nodes, k) = node;                                               \
        return &kh_val(nodes, k);                                              \
    }                                                                          \
                                                                               \
    uint32_t map_##tag##_remove(map_##tag##_s* nodes, const char* serial)      \
    {                                                                          \
        khiter_t k;                                                            \
        type* d;                                                               \
        uint32_t count = 0;                                                    \
        if (!((k = kh_get_##tag(nodes, serial)) == kh_end(nodes))) {           \
            d = kh_val(nodes, k);                                              \
            kh_del_##tag(nodes, k);                                            \
            map_free_fn(&d);                                                   \
            count = 1;                                                         \
        }                                                                      \
        return count;                                                          \
    }                                                                          \
                                                                               \
    type** map_##tag##_get(map_##tag##_s* hash, const char* serial)            \
    {                                                                          \
        khiter_t k;                                                            \
        return ((k = kh_get_##tag(hash, serial)) == kh_end(hash))              \
                   ? NULL                                                      \
                   : &kh_val(hash, k);                                         \
    }                                                                          \
                                                                               \
    uint32_t map_##tag##_size(map_##tag##_s* hash) { return kh_size(hash); }   \
                                                                               \
    void map_##tag##_foreach(                                                  \
        map_##tag##_s* hash, map_##tag##_foreach_fn fn, void* ctx)             \
    {                                                                          \
        for (khiter_t k = kh_begin(hash); k != kh_end(hash); ++k) {            \
            if (kh_exist(hash, k)) fn(ctx, &kh_val(hash, k));                  \
        }                                                                      \
    }

#ifdef __cplusplus
}
#endif
#endif
