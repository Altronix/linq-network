#ifndef RLP_H
#define RLP_H

#include "stdint.h"
#include "string.h"

// clang-format off
#if defined _WIN32
#  define rlp_assert_fn assert
#  define rlp_malloc_fn malloc
#  define rlp_free_fn free
#  define rlp_clz_fn __lzcnt
#  if defined RLP_STATIC
#    define RLP_EXPORT
#  elif defined DLL_EXPORT
#    define RLP_EXPORT __declspec(dllexport)
#  else
#    define RLP_EXPORT __declspec(dllimport)
#  endif
#else
#  define rlp_assert_fn assert
#  define rlp_malloc_fn malloc
#  define rlp_free_fn free
#  define rlp_clz_fn __builtin_clz
#  define RLP_EXPORT
#endif
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct rlp rlp; /*!< opaque class */
    typedef void (*rlp_walk_fn)(const rlp*, int, void*);

#ifndef RLP_CONFIG_ANYSIZE_ARRAY
#define RLP_CONFIG_ANYSIZE_ARRAY 1
#endif

#ifndef RLP_CONFIG_MAX_ARRAY
#define RLP_CONFIG_MAX_ARRAY 4098
#endif

#define rlp_item(b) rlp_item_str(b)   /*!< alias */
#define rlp_is_list(rlp) (!(rlp->sz)) /*!< empty node signal start of list */

    rlp* rlp_alloc(uint32_t);
    void rlp_free(rlp**);
    uint32_t rlp_read_size(const uint8_t* b);
    rlp* rlp_list();
    rlp* rlp_item_u64(uint64_t);
    rlp* rlp_item_u32(uint32_t);
    rlp* rlp_item_u16(uint16_t);
    rlp* rlp_item_u8(uint8_t);
    rlp* rlp_item_u64_arr(const uint64_t*, uint32_t sz);
    rlp* rlp_item_u32_arr(const uint32_t*, uint32_t sz);
    rlp* rlp_item_u16_arr(const uint16_t*, uint32_t sz);
    rlp* rlp_item_u8_arr(const uint8_t*, uint32_t);
    rlp* rlp_item_str(const char*);
    rlp* rlp_item_mem(const uint8_t* b, uint32_t l);
    int rlp_idx_to_u64(const rlp*, uint32_t idx, uint64_t* val);
    int rlp_idx_to_u32(const rlp*, uint32_t idx, uint32_t* val);
    int rlp_idx_to_u16(const rlp*, uint32_t idx, uint16_t* val);
    int rlp_idx_to_u8(const rlp*, uint32_t idx, uint8_t* val);
    int rlp_idx_to_mem(const rlp*, uint32_t idx, uint8_t* mem, uint32_t* l);
    int rlp_idx_to_str(const rlp*, uint32_t idx, char* str);
    uint64_t rlp_unsafe_idx_as_u64(const rlp*, uint32_t idx);
    uint32_t rlp_unsafe_idx_as_u32(const rlp*, uint32_t idx);
    uint16_t rlp_unsafe_idx_as_u16(const rlp*, uint32_t idx);
    uint8_t rlp_unsafe_idx_as_u8(const rlp*, uint32_t idx);
    const uint8_t* rlp_unsafe_idx_as_mem(const rlp*, uint32_t idx);
    const char* rlp_unsafe_idx_as_str(const rlp*, uint32_t idx);
    uint64_t rlp_as_u64(const rlp*);
    uint32_t rlp_as_u32(const rlp*);
    uint16_t rlp_as_u16(const rlp*);
    uint8_t rlp_as_u8(const rlp*);
    const char* rlp_as_str(const rlp* rlp);
    const uint8_t* rlp_as_mem(const rlp*, uint32_t*);
    const uint8_t* rlp_ref(const rlp*, uint32_t*);
    rlp* rlp_copy(const rlp*);
    int rlp_read_int(const rlp*, void* m, uint32_t);
    const rlp* rlp_at(const rlp*, uint32_t);
    rlp* rlp_push(rlp*, rlp*);
    uint32_t rlp_size_rlp(const rlp* rlp);
    uint32_t rlp_size(const rlp*);
    const uint8_t* rlp_data(rlp* rlp);
    const rlp* rlp_child(const rlp* rlp);
    uint32_t rlp_children(const rlp* rlp);
    uint32_t rlp_children_walk(const rlp* rlp);
    uint32_t rlp_siblings(const rlp* rlp);
    uint32_t rlp_print_size(const rlp* rlp);
    int rlp_print(const rlp*, uint8_t* b, uint32_t* sz);
    rlp* rlp_parse(const uint8_t* b, uint32_t);
    void rlp_foreach(const rlp*, void* ctx, rlp_walk_fn fn);

    // Can macro these
    static inline int rlp_push_u8(rlp* r, uint8_t b)
    {
        rlp* item = rlp_item_u8(b);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u16(rlp* r, uint16_t b)
    {

        rlp* item = rlp_item_u16(b);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u32(rlp* r, uint32_t b)
    {
        rlp* item = rlp_item_u32(b);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u64(rlp* r, uint64_t b)
    {
        rlp* item = rlp_item_u64(b);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u8_arr(rlp* r, const uint8_t* b, uint32_t sz)
    {
        rlp* item = rlp_item_u8_arr(b, sz);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u16_arr(rlp* r, const uint16_t* b, uint32_t sz)
    {
        rlp* item = rlp_item_u16_arr(b, sz);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u32_arr(rlp* r, const uint32_t* b, uint32_t sz)
    {
        rlp* item = rlp_item_u32_arr(b, sz);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_u64_arr(rlp* r, const uint64_t* b, uint32_t sz)
    {
        rlp* item = rlp_item_u64_arr(b, sz);
        if (item) {
            rlp_push(r, item);
            return 0;
        }
        return -1;
    }

    static inline int rlp_push_mem(rlp* r, const uint8_t* b, uint32_t sz)
    {
        return rlp_push_u8_arr(r, b, sz);
    }

    static inline int rlp_push_str(rlp* r, const char* str)
    {
        return rlp_push_u8_arr(r, (const uint8_t*)str, strlen(str));
    }

#ifdef __cplusplus
}
#endif
#endif /* RLP_H */
