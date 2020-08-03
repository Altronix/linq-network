// Copyright 2017 Altronix Corp.
// This file is part of the tiny-ether library
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @author Thomas Chiantia <thomas@altronix>
 * @date 2017
 */

/**
 * @file rlp.c
 *
 * @brief Encode and decode bytes and lists of bytes into rlp format.
 *
 * @code
 * 	uint8_t rlp_bytes[100]; ///Some stack for encoded rlp
 *
 * 	// ["cat","dog"] - rlp some pets
 * 	rlp *list = rlp_list();
 * 	rlp_push(list,"cat");
 * 	rlp_push(list,"dog");
 *
 *	// print out rlp  (0xc8, 0x83, 'c', 'a', 't', 0x83, ...)
 * 	rlp_print(list,rlp_bytes,100);
 * 	printf("%s\n",rlp_bytes);
 *
 *	// free heap
 *	rlp_free(&list);
 */

#include "rlp.h"
#include "stdlib.h"

/**
 * @brief rlp context
 * extra padding b[] for node rlp.
 */
typedef struct rlp
{
    struct rlp *next, *child; /*!< list pointers FIFO */
    uint32_t n;               /*!< Number of children */
    uint32_t sz;              /*!< Number of bytes of rlp */
    uint8_t b[];              /*!< Bytes of RLP stored here */
} rlp;

// private
uint32_t rlp_szsz(uint32_t); // size of size
uint32_t rlp_write_sz(uint8_t* b, uint32_t* s, uint32_t sz, int islist);
uint32_t rlp_write_n_big_endian(uint8_t*, const void*, uint32_t, int);
uint32_t rlp_write_big_endian(uint8_t*, const void*, int);
uint32_t rlp_read_sz(const uint8_t* b, uint32_t* result);
uint32_t rlp_print_walk(const rlp* rlp, uint8_t* b, uint32_t* spot);
rlp* rlp_parse_walk(const uint8_t* b, uint32_t l);

rlp*
rlp_alloc(uint32_t sz)
{
    rlp* r = NULL;
    r = rlp_malloc_fn(sizeof(rlp) + RLP_CONFIG_ANYSIZE_ARRAY + sz);
    if (r) {
        memset(r, 0, sizeof(rlp) + RLP_CONFIG_ANYSIZE_ARRAY + sz);
        r->sz = sz;
    }
    return r;
}

void
rlp_free(rlp** rlp_p)
{
    rlp* r = *rlp_p;
    *rlp_p = NULL;
    while (r) {
        rlp* delete = r;
        r = r->next;
        if (delete->child) rlp_free(&delete->child);
        rlp_free_fn(delete);
    }
}

uint32_t
rlp_read_size(const uint8_t* b)
{
    uint32_t result, szsz;
    szsz = rlp_read_sz(b, &result);
    return result + szsz;
}

uint32_t
rlp_szsz(uint32_t size)
{
    return 4 - (rlp_clz_fn(size) / 8);
}

uint32_t
rlp_write_sz(uint8_t* b, uint32_t* c, uint32_t s, int islist)
{
    uint32_t sz = 0, szsz = 0;
    if (s < 1) {
        sz = 0;
    } else if (s <= 55) {
        if (b) b[--*(c)] = s + (islist ? 0xc0 : 0x80);
        sz = 1;
    } else {
        szsz = rlp_szsz(s);
        if (b) {
            *c -= szsz;
            rlp_write_big_endian(&b[*c], &s, 4);
            b[--*c] = szsz + (islist ? 0xf7 : 0xb7);
        }
        sz = szsz + 1;
    }
    return sz;
}

uint32_t
rlp_write_n_big_endian(uint8_t* b, const void* dat, uint32_t len, int szof)
{
    uint32_t spot = 0, n;
    while (len--) {
        n = rlp_write_big_endian(&b[spot], dat, szof);
        dat += szof;
        spot += n;
    }
    return spot;
}

uint32_t
rlp_write_big_endian(uint8_t* b, const void* dat, int szof)
{
    //[0x01,0x00,0x00,0x00] uint32_t int = 1; // little endian
    //[0x00,0x00,0x00,0x01] uint32_t int = 1; // big endian
    static int test = 1;                /*!< endianess test */
    uint8_t* x = (&((uint8_t*)dat)[0]); /*!< inner bytes ptr */
    int inc = 1;                        /*!< ptr(++/--) */
    uint32_t c = 0;                     /*!< Bytes written */
    int hit = 0;                        /*!< start writing bytes */
    if (*(char*)&test) {                /*!< if little endian (start at end) */
        x = (&((uint8_t*)dat)[szof - 1]);
        inc = -1;
    }
    hit = 0;
    while (szof--) {
        if (*x) hit = 1;
        // If last byte, or already set msb or *x has weight.
        if (hit || *x || !szof) {
            if (b) b[c] = *x;
            c++;
        }
        x += inc;
    }
    return c;
}

uint32_t
rlp_read_big_endian(void* dat, int szof, const uint8_t* b)
{
    static int test = 1;
    uint8_t* x = (&((uint8_t*)dat)[szof - 1]);
    int inc = -1;
    if (*(char*)&test == 0) { /*!< if we are big endian, read into mem.*/
        memcpy(dat, b, szof);
        return szof;
    }
    while (szof--) {
        *x = *b++;
        x += inc;
    }
    return szof;
}

uint32_t
rlp_read_sz(const uint8_t* b, uint32_t* result)
{
    uint32_t sz = 0, szsz = 0;
    *result = 0;
    if (*b < 0x80) {
        *result = 1;
        sz = 0;
    } else if (*b <= 0xb7) {
        *result = *b - 0x80;
        sz = 1;
    } else if (*b <= 0xbf) {
        szsz = *b - 0xb7;
        rlp_read_big_endian(&sz, szsz, ++b);
        *result = sz;
        sz = 1 + szsz;
    } else if (*b == 0xc0) {
        *result = 1;
        sz = 0;
    } else if (*b <= 0xf7) {
        *result = *b - 0xc0;
        sz = 1;
    } else {
        szsz = *b - 0xf7;
        rlp_read_big_endian(&sz, szsz, ++b);
        *result = sz;
        sz = 1 + szsz;
    }
    return sz;
}

rlp*
rlp_list()
{
    return rlp_alloc(0); //
}

rlp*
rlp_item_u64(uint64_t val)
{
    return rlp_item_u64_arr(&val, 1);
}

rlp*
rlp_item_u32(uint32_t val)
{
    return rlp_item_u32_arr(&val, 1);
}

rlp*
rlp_item_u16(uint16_t val)
{
    return rlp_item_u16_arr(&val, 1);
}

rlp*
rlp_item_u8(uint8_t val)
{
    return rlp_item_u8_arr(&val, 1);
}

rlp*
rlp_item_u64_arr(const uint64_t* b, uint32_t sz)
{
    uint32_t blen = sz * sizeof(uint64_t); // worstcase
    uint8_t bytes[blen];
    uint32_t len = rlp_write_n_big_endian(bytes, b, sz, sizeof(uint64_t));
    return rlp_item_u8_arr(bytes, len);
}

rlp*
rlp_item_u32_arr(const uint32_t* b, uint32_t sz)
{
    uint32_t blen = sz * sizeof(uint32_t); // worstcase
    uint8_t bytes[blen];
    uint32_t len = rlp_write_n_big_endian(bytes, b, sz, sizeof(uint32_t));
    return rlp_item_u8_arr(bytes, len);
}

rlp*
rlp_item_u16_arr(const uint16_t* b, uint32_t sz)
{
    uint32_t blen = sz * sizeof(uint16_t); // worstcase
    uint8_t bytes[blen];
    uint32_t len = rlp_write_n_big_endian(bytes, b, sz, sizeof(uint16_t));
    return rlp_item_u8_arr(bytes, len);
}

rlp*
rlp_item_u8_arr(const uint8_t* b, uint32_t sz)
{
    rlp* rlp = NULL;
    uint32_t size;
    if (sz == 0) {
        size = 1;
        rlp = rlp_alloc(size);
        if (rlp) rlp->b[--size] = 0x80;
    } else if (sz == 1 && b[0] < 0x80) {
        size = 1;
        rlp = rlp_alloc(size);
        if (rlp) rlp->b[--size] = b[0];
    } else if (sz <= 55) {
        size = sz + 1;
        rlp = rlp_alloc(size);
        if (rlp) {
            for (int i = sz; i; i--) rlp->b[--size] = b[i - 1];
            rlp->b[--size] = 0x80 + sz;
        }
    } else {
        size = rlp_szsz(sz) + 1 + sz; // prefix + size of size + string
        rlp = rlp_alloc(size);
        if (rlp) {
            for (int i = sz; i; i--) rlp->b[--size] = b[i - 1];
            rlp_write_sz(rlp->b, &size, sz, 0);
        }
    }
    return rlp;
}

rlp*
rlp_item_str(const char* b)
{
    return rlp_item_mem((const uint8_t*)b, strlen(b));
}

rlp*
rlp_item_mem(const uint8_t* b, uint32_t sz)
{
    return rlp_item_u8_arr((uint8_t*)b, sz);
}

int
rlp_idx_to_u64(const rlp* rlp, uint32_t idx, uint64_t* val)
{
    rlp = rlp_at(rlp, idx);
    if (rlp) {
        *val = rlp_as_u64(rlp);
        return 0;
    } else {
        return -1;
    }
}

int
rlp_idx_to_u32(const rlp* rlp, uint32_t idx, uint32_t* val)
{
    rlp = rlp_at(rlp, idx);
    if (rlp) {
        *val = rlp_as_u32(rlp);
        return 0;
    } else {
        return -1;
    }
}

int
rlp_idx_to_u16(const rlp* rlp, uint32_t idx, uint16_t* val)
{
    rlp = rlp_at(rlp, idx);
    if (rlp) {
        *val = rlp_as_u16(rlp);
        return 0;
    } else {
        return -1;
    }
}

int
rlp_idx_to_u8(const rlp* rlp, uint32_t idx, uint8_t* val)
{
    rlp = rlp_at(rlp, idx);
    if (rlp) {
        *val = rlp_as_u8(rlp);
        return 0;
    } else {
        return -1;
    }
}

int
rlp_idx_to_mem(const rlp* rlp, uint32_t idx, uint8_t* mem, uint32_t* l)
{
    rlp = rlp_at(rlp, idx);
    const uint8_t* ref;
    uint32_t len;
    if (rlp) {
        ref = rlp_as_mem(rlp, &len);
        if (len <= *l) {
            memcpy(mem, ref, len);
            *l = len;
            return 0;
        } else {
            *l = len;
            return -1;
        }
    } else {
        return -1;
    }
}

int
rlp_idx_to_str(const rlp* rlp, uint32_t idx, char* str)
{
    rlp = rlp_at(rlp, idx);
    const uint8_t* ref;
    uint32_t len;
    if (rlp) {
        ref = rlp_as_mem(rlp, &len);
        memcpy(str, ref, len);
        return 0;
    } else {
        return -1;
    }
}

uint64_t
rlp_unsafe_idx_as_u64(const rlp* rlp, uint32_t idx)
{
    return rlp_as_u64(rlp_at(rlp, idx));
}

uint32_t
rlp_unsafe_idx_as_u32(const rlp* rlp, uint32_t idx)
{
    return rlp_as_u32(rlp_at(rlp, idx));
}

uint16_t
rlp_unsafe_idx_as_u16(const rlp* rlp, uint32_t idx)
{
    return rlp_as_u16(rlp_at(rlp, idx));
}

uint8_t
rlp_unsafe_idx_as_u8(const rlp* rlp, uint32_t idx)
{
    return rlp_as_u8(rlp_at(rlp, idx));
}

const uint8_t*
rlp_unsafe_idx_as_mem(const rlp* rlp, uint32_t idx)
{
    return rlp_as_mem(rlp_at(rlp, idx), NULL);
}

const char*
rlp_unsafe_idx_as_str(const rlp* rlp, uint32_t idx)
{
    return rlp_as_str(rlp_at(rlp, idx));
}

uint64_t
rlp_as_u64(const rlp* rlp)
{
    uint64_t ret = 0;
    return rlp_read_int(rlp, &ret, sizeof(uint64_t)) == 1 ? ret : 0;
}

uint32_t
rlp_as_u32(const rlp* rlp)
{
    uint32_t ret = 0;
    return rlp_read_int(rlp, &ret, sizeof(uint32_t)) == 1 ? ret : 0;
}

uint16_t
rlp_as_u16(const rlp* rlp)
{
    uint16_t ret = 0;
    return rlp_read_int(rlp, &ret, sizeof(uint16_t)) == 1 ? ret : 0;
}

uint8_t
rlp_as_u8(const rlp* rlp)
{
    uint8_t ret = 0;
    return rlp_read_int(rlp, &ret, sizeof(uint8_t)) == 1 ? ret : 0;
}

const char*
rlp_as_str(const rlp* rlp)
{
    return (const char*)rlp_ref(rlp, NULL);
}

const uint8_t*
rlp_as_mem(const rlp* rlp, uint32_t* sz)
{
    return rlp_ref(rlp, sz);
}

const uint8_t*
rlp_ref(const rlp* rlp, uint32_t* sz)
{
    uint32_t l = 0;
    if (!sz) sz = &l; // caller doesn't care about length so passed NULL
    const uint8_t* b = rlp->sz ? rlp->b + rlp_read_sz(rlp->b, sz) : NULL;
    if (!b) *sz = 0;
    return b;
}

rlp*
rlp_copy(const rlp* rlp)
{
    uint32_t sz = rlp_print_size(rlp);
    uint8_t buf[sz];
    return rlp_print(rlp, buf, &sz) ? NULL : rlp_parse(buf, sz);
}

int
rlp_read_int(const rlp* rlp, void* mem, uint32_t szof)
{
    uint32_t n;
    const uint8_t* b = rlp_ref(rlp, &n);
    if (!(b && n <= szof)) return -1;
    rlp_read_big_endian(mem, n < szof ? n : szof, b);
    return 1; // TODO needs fix
    return 0;
}

const rlp*
rlp_at(const rlp* rlp, uint32_t where)
{
    uint32_t n = rlp->n - (where + 1);
    rlp = rlp->child;
    while (rlp && n--) rlp = rlp->next;
    return rlp;
}

rlp*
rlp_push(rlp* parent, rlp* child)
{
    if (!parent) {
        parent = rlp_alloc(0);
    } else if (!rlp_is_list(parent)) {
        // first item in list always start with sz=0 node.
        // Note that we are changing the root node because caller is turning
        // a single item into a list... Might need better api to distingish this
        // to caller. We always return parent node to caller, but their original
        // callers parent node is now different. For safety we should not allow
        // this, and require caller to be more explicit when creating list or an
        // item... ie: if (!rlp_is_list(parent))return NULL; ...
        // Right now this code supports turning single items into list for them.
        parent = rlp_push(rlp_list(), parent);
    }
    if (parent->child) {
        child->next = parent->child;
        parent->child = child;
    } else {
        parent->child = child;
    }
    parent->n++;
    return parent;
}

uint32_t
rlp_size_rlp(const rlp* rlp)
{
    return rlp->sz; //
}

uint32_t
rlp_size(const rlp* rlp)
{
    uint32_t sz;
    return (rlp_ref(rlp, &sz)) ? sz : 0;
}

const uint8_t*
rlp_data(rlp* rlp)
{
    return rlp->b; //
}

const rlp*
rlp_child(const rlp* rlp)
{
    return rlp->child;
}

uint32_t
rlp_children(const rlp* rlp)
{
    uint32_t n;
    if (rlp_is_list(rlp)) {
        // n = rlp->n + rlp_children_walk(rlp->child);
        return rlp->n;
    } else {
        n = 0;
    }
    return n;
}

uint32_t
rlp_children_walk(const rlp* rlp)
{
    uint32_t n = 0;
    while (rlp) {
        if (rlp_is_list(rlp)) { n = rlp->n + rlp_children_walk(rlp->child); }
        rlp = rlp->next;
    }
    return n;
}

uint32_t
rlp_siblings(const rlp* rlp)
{
    uint32_t n = 0;
    while (rlp) {
        n++;
        rlp = rlp->next;
    }
    return n;
}

uint32_t
rlp_print_size(const rlp* rlp)
{
    return rlp_print_walk(rlp->child, NULL, 0);
}

int
rlp_print(const rlp* rlp, uint8_t* b, uint32_t* l)
{
    int err = -1;
    uint32_t sz, spot;
    if (!rlp_is_list(rlp)) {
        // handle case where this is single item and not a list
        if (rlp->sz <= *l) {
            if (b) {
                for (int i = rlp->sz - 1; i >= 0; i--) b[i] = rlp->b[i];
            }
            err = 0;
        }
        *l = rlp->sz;
    } else {
        if (rlp->child) {
            // Regular list
            spot = sz = rlp_print_walk(rlp->child, NULL, 0); // get size
            if (sz <= *l) {
                rlp_print_walk(rlp->child, b, &spot); // print if ok
                err = 0;
            }
            *l = sz;
        } else {
            // We have empty list... []
            if (*l) {
                if (b) *b = 0xc0;
                err = 0;
            }
            *l = 1;
        }
    }
    return err;
}

uint32_t
rlp_print_walk(const rlp* rlp, uint8_t* b, uint32_t* spot)
{
    uint32_t sz = 0;
    while (rlp) {
        if (rlp_is_list(rlp)) {
            if (rlp->child) {
                sz += rlp_print_walk(rlp->child, b, spot);
            } else { // empty list;
                sz += 1;
                if (b) b[--*(spot)] = 0xc0;
            }
        }
        if (b) {
            uint32_t rlpsz = rlp->sz;
            while (rlpsz) b[--*(spot)] = rlp->b[--rlpsz];
        }
        sz += rlp->sz;
        rlp = rlp->next;
    }
    sz += rlp_write_sz(b, spot, sz, 1);
    return sz;
}

rlp*
rlp_parse(const uint8_t* b, uint32_t l)
{
    rlp* rlp = NULL;
    uint32_t sz = 0;
    if (!b) return NULL;
    if (*b < 0xc0) {
        // Handle case where this is a single item and not a list
        uint32_t sz;
        b += rlp_read_sz(b, &sz);
        rlp = rlp_item_u8_arr(b, sz);
    } else {
        if (*b > 0xc0) {
            // regular list
            b += rlp_read_sz(b, &sz);
            rlp = rlp_parse_walk(b, sz);
        } else {
            // empty list []
            return rlp_list();
        }
    }
    return rlp;
}

rlp*
rlp_parse_walk(const uint8_t* b, uint32_t l)
{
    rlp* rlp = NULL;
    const uint8_t* end = &b[l];
    uint32_t sz;
    while (b < end) {
        if (*b >= 0xc0) {
            // This is a list.
            if (*b == 0xc0) {
                // Push empty list
                rlp = rlp_push(rlp, rlp_list());
                b++;
            } else {
                // Push list of items into our list (recursive.)
                b += rlp_read_sz(b, &sz);
                rlp = rlp_push(rlp, rlp_parse_walk(b, sz));
                b += sz;
            }
        } else {
            // This is an item.
            b += rlp_read_sz(b, &sz);
            rlp = rlp_push(rlp, rlp_item_u8_arr(b, sz));
            b += sz;
        }
    }
    return rlp;
}

void
rlp_foreach(const rlp* rlp, void* ctx, rlp_walk_fn fn)
{
    uint32_t n;
    if (rlp) {
        rlp = rlp_is_list(rlp) ? rlp->child : NULL;
        n = rlp_siblings(rlp);
    }
    while (rlp) {
        fn(rlp, --n, ctx);
        rlp = rlp->next;
    }
}

//
//
//
//

