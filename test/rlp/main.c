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

#include "common/rlp.h"
#include "stdio.h"

uint8_t rlp_null[] = { '\x80' };
uint8_t rlp_null2[] = { '\xc2', '\x80', '\x80' };
uint8_t rlp_15[] = { '\x0f' };
uint8_t rlp_1024[] = { '\x82', '\x04', '\x00' };
uint8_t rlp_empty[] = { '\xc0' };
uint8_t rlp_empty_empty[] = { '\xc2', '\xc0', '\xc0' };
uint8_t rlp_empty_nest[] = { '\xc2', '\xc1', '\xc0' };
uint8_t rlp_cat[] = { '\x83', 'c', 'a', 't' };
uint8_t rlp_dog[] = { '\x83', 'd', 'o', 'g' };
uint8_t rlp_catdog[] = { '\xc8', '\x83', 'c', 'a', 't', '\x83', 'd', 'o', 'g' };
uint8_t rlp_max64[] = { '\x88', '\xff', '\xff', '\xff', '\xff',
                        '\xff', '\xff', '\xff', '\xff' };
uint8_t rlp_half64[] = { '\x88', '\x01', '\x00', '\x00', '\x00',
                         '\x00', '\x00', '\x00', '\x00' };
uint8_t rlp_max32[] = { '\x84', '\xff', '\xff', '\xff', '\xff' };
uint8_t rlp_half32[] = { '\x84', '\x01', '\x00', '\x00', '\x00' };
uint8_t rlp_max16[] = { '\x82', '\xff', '\xff' };
uint8_t rlp_half16[] = { '\x82', '\x01', '\x00' };
uint8_t rlp_types[] = {
    '\xd7',                                                                 //
    '\x83', 'c',    'a',    't',                                            //
    '\x88', '\xaa', '\xbb', '\xcc', '\xdd', '\xaa', '\xbb', '\xcc', '\xdd', //
    '\x84', '\xaa', '\xbb', '\xcc', '\xdd',                                 //
    '\x82', '\xaa', '\xbb',                                                 //
    '\x81', '\xaa'                                                          //
};
uint8_t rlp_catdogpig[] = {
    '\xcc',                //
    '\x83', 'c', 'a', 't', //
    '\x83', 'd', 'o', 'g', //
    '\x83', 'p', 'i', 'g'  //
};
uint8_t rlp_catdogpigcow[] = {
    '\xd2',                //
    '\xc8',                //
    '\x83', 'c', 'a', 't', //
    '\x83', 'd', 'o', 'g', //
    '\xc8',                //
    '\x83', 'p', 'i', 'g', //
    '\x83', 'c', 'o', 'w'  //
};
uint8_t rlp_lorem[] = {
    '\xb8', '\x38', 'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p', 's', 'u',
    'm',    ' ',    'd', 'o', 'l', 'o', 'r', ' ', 's', 'i', 't', ' ',
    'a',    'm',    'e', 't', ',', ' ', 'c', 'o', 'n', 's', 'e', 'c',
    't',    'e',    't', 'u', 'r', ' ', 'a', 'd', 'i', 'p', 'i', 's',
    'i',    'c',    'i', 'n', 'g', ' ', 'e', 'l', 'i', 't' //
};

// TODO
uint8_t rlp_2lorem[] = {
    '\xb8', '\x38', 'L', 'o', 'r', 'e', 'm',    ' ',    'i', 'p', 's', 'u', 'm',
    ' ',    'd',    'o', 'l', 'o', 'r', ' ',    's',    'i', 't', ' ', 'a', 'm',
    'e',    't',    ',', ' ', 'c', 'o', 'n',    's',    'e', 'c', 't', 'e', 't',
    'u',    'r',    ' ', 'a', 'd', 'i', 'p',    'i',    's', 'i', 'c', 'i', 'n',
    'g',    ' ',    'e', 'l', 'i', 't', '\xb8', '\x38', 'L', 'o', 'r', 'e', 'm',
    ' ',    'i',    'p', 's', 'u', 'm', ' ',    'd',    'o', 'l', 'o', 'r', ' ',
    's',    'i',    't', ' ', 'a', 'm', 'e',    't',    ',', ' ', 'c', 'o', 'n',
    's',    'e',    'c', 't', 'e', 't', 'u',    'r',    ' ', 'a', 'd', 'i', 'p',
    'i',    's',    'i', 'c', 'i', 'n', 'g',    ' ',    'e', 'l', 'i', 't' //
};

uint8_t rlp_random[] = {
    '\xe1',                                               // [...
    '\x83', 'c',    'a', 't',                             // "cat"
    '\xc8', '\x83', 'c', 'a', 't', '\x83', 'd', 'o', 'g', // ["cat","dog"]
    '\x85', 'h',    'o', 'r', 's', 'e',                   // "horse"
    '\xc1', '\xc0',                                       // [[]]
    '\x83', 'p',    'i', 'g',                             // "pig"
    '\xc1', '\x80',                                       // [""]
    '\x85', 's',    'h', 'e', 'e', 'p'                    // "sheep"
};
uint8_t rlp_wat[] = { '\xc7', '\xc0', '\xc1', '\xc0',
                      '\xc3', '\xc0', '\xc1', '\xc0' };

int test_push_types();
int test_conversions();
int test_foreach();
int test_copy();
int test_u8();
int test_u16();
int test_u32();
int test_u64();
int test_item(uint8_t*, uint32_t, rlp**);
void test_walk_fn(const rlp* rlp, int idx, void* ctx);

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = 0;
    err |= test_push_types();
    err |= test_foreach();
    err |= test_copy();
    err |= test_u8();
    err |= test_u16();
    err |= test_u32();
    err |= test_u64();
    printf("%s\n", err ? "\x1b[91m[ERR]\x1b[0m" : "\x1b[32m[ OK]\x1b[0m");
    return err;
}

int
test_push_types()
{
    int err = 0;
    uint64_t u64 = 0xaabbccddaabbccdd;
    uint32_t u32 = 0xaabbccdd;
    uint16_t u16 = 0xaabb;
    uint8_t u8 = 0xaa;
    rlp* rlp = rlp_list();
    err |= rlp_push_str(rlp, "cat");
    err |= rlp_push_u64_arr(rlp, &u64, 1);
    err |= rlp_push_u32_arr(rlp, &u32, 1);
    err |= rlp_push_u16_arr(rlp, &u16, 1);
    err |= rlp_push_u8_arr(rlp, &u8, 1);
    err |= test_item(rlp_types, sizeof(rlp_types), &rlp);
    return err;
}

int
test_conversions()
{
    uint32_t memlen = 3;
    uint8_t mem[memlen];
    uint64_t tu64;
    uint32_t tu32;
    uint16_t tu16;
    uint8_t tu8;
    const uint8_t* cmem;
    const uint64_t ctu64;
    const uint32_t ctu32;
    const uint16_t ctu16;
    const uint8_t ctu8;
    rlp* rlp = rlp_list();
    rlp_push(rlp, rlp_item_mem((uint8_t*)"\x03\x02\x01", 3));
    rlp_push(rlp, rlp_item_str("hello world"));
    // TODO - need to test conversion
    // rlp_idx_to_..
    // rlp_unsafe_idx_as_...
    return 0;
}

int
test_foreach()
{
    uint32_t mask = 0;
    rlp* rlp = rlp_list();
    rlp_push(rlp, rlp_item_str("zero"));
    rlp_push(rlp, rlp_item_str("one"));
    rlp_push(rlp, rlp_item_str("two"));
    rlp_foreach(rlp, &mask, test_walk_fn);
    rlp_free(&rlp);
    return mask == (0b111) ? 0 : -1;
}

void
test_walk_fn(const rlp* rlp, int idx, void* ctx)
{
    uint32_t *mask_ptr = (uint32_t*)ctx, len = 5;
    uint8_t print[len];
    rlp_print(rlp, print, &len);
    if (idx == 0) {
        if (!memcmp(print, "\x84zero", 5)) *mask_ptr |= 0x01 << idx;
    } else if (idx == 1) {
        if (!memcmp(print, "\x83one", 4)) *mask_ptr |= 0x01 << idx;
    } else if (idx == 2) {
        if (!memcmp(print, "\x83two", 4)) *mask_ptr |= 0x01 << idx;
    }
}

int
test_copy()
{
    int err;
    rlp *cpy = NULL, *wat = rlp_parse(rlp_wat, sizeof(rlp_wat));
    cpy = rlp_copy(wat);
    err = test_item(rlp_wat, sizeof(rlp_wat), &cpy);
    rlp_free(&wat);
    return err;
}

int
test_u8()
{
    int err = 0;
    char* lorem = "Lorem ipsum dolor sit amet, consectetur adipisicing elit";
    rlp* rlp;

    // ""
    rlp = rlp_item("");
    err |= test_item(rlp_null, sizeof(rlp_null), &rlp);

    // ["",""]
    rlp = rlp_list();
    rlp_push(rlp, rlp_item(""));
    rlp_push(rlp, rlp_item(""));
    err |= test_item(rlp_null2, sizeof(rlp_null2), &rlp);

    // 0x0f
    rlp = rlp_item_mem((uint8_t*)"\x0f", 1);
    err |= rlp_as_u8(rlp) == 15 ? 0 : -1;
    err |= rlp_as_u16(rlp) == 15 ? 0 : -1;
    err |= rlp_as_u32(rlp) == 15 ? 0 : -1;
    err |= rlp_as_u64(rlp) == 15 ? 0 : -1;
    err |= test_item(rlp_15, sizeof(rlp_15), &rlp);

    // 0x400x00
    rlp = rlp_item_mem((uint8_t*)"\x04\x00", 2);
    err |= rlp_as_u16(rlp) == 1024 ? 0 : -1;
    err |= rlp_as_u32(rlp) == 1024 ? 0 : -1;
    err |= rlp_as_u64(rlp) == 1024 ? 0 : -1;
    err |= test_item(rlp_1024, sizeof(rlp_1024), &rlp);

    // "cat"
    rlp = rlp_item("cat");
    err |= memcmp(rlp_as_str(rlp), "cat", 3) ? -1 : 0;
    err |= test_item(rlp_cat, sizeof(rlp_cat), &rlp);

    // "dog"
    rlp = rlp_item("dog");
    err |= test_item(rlp_dog, sizeof(rlp_dog), &rlp);

    // "lorem...
    rlp = rlp_item(lorem);
    err |= test_item(rlp_lorem, sizeof(rlp_lorem), &rlp);

    // []
    rlp = rlp_list();
    err |= test_item(rlp_empty, sizeof(rlp_empty), &rlp);

    // [[],[]]
    rlp = rlp_list();
    rlp_push(rlp, rlp_list());
    rlp_push(rlp, rlp_list());
    err |= test_item(rlp_empty_empty, sizeof(rlp_empty_empty), &rlp);

    // [[[]]]
    rlp = rlp_push(rlp_list(), rlp_push(rlp_list(), rlp_list()));
    err |= test_item(rlp_empty_nest, sizeof(rlp_empty_nest), &rlp);

    //[[],[[]],[ [],[[]] ] ]
    rlp = rlp_list();
    rlp_push(rlp, rlp_list());
    rlp_push(rlp, rlp_push(rlp_list(), rlp_list()));
    rlp_push(
        rlp,
        rlp_push(
            rlp_push(rlp_list(), rlp_list()), //
            rlp_push(rlp_list(), rlp_list())) //
    );
    err |= (rlp_siblings(rlp_child(rlp)) == 3 ? 0 : -1);
    err |= test_item(rlp_wat, sizeof(rlp_wat), &rlp);

    // ["cat","dog"]
    rlp = rlp_push(rlp_item("cat"), rlp_item("dog"));
    err |= test_item(rlp_catdog, sizeof(rlp_catdog), &rlp);

    // ["cat","dog","pig"]
    rlp = rlp_item("cat");
    rlp = rlp_push(rlp, rlp_item("dog"));
    rlp = rlp_push(rlp, rlp_item("pig"));
    err |= memcmp(rlp_as_str(rlp_at(rlp, 0)), "cat", 3) ? -1 : 0;
    err |= memcmp(rlp_as_str(rlp_at(rlp, 1)), "dog", 3) ? -1 : 0;
    err |= memcmp(rlp_as_str(rlp_at(rlp, 2)), "pig", 3) ? -1 : 0;
    err |= test_item(rlp_catdogpig, sizeof(rlp_catdogpig), &rlp);

    // [["cat","dog"],["pig","cow"]]
    rlp = rlp_list();
    rlp_push(rlp, rlp_push(rlp_item("cat"), rlp_item("dog")));
    rlp_push(rlp, rlp_push(rlp_item("pig"), rlp_item("cow")));
    err |= test_item(rlp_catdogpigcow, sizeof(rlp_catdogpigcow), &rlp);

    // ["cat",["cat","dog"],"horse",[[]],"pig",[""],"sheep"]
    rlp = rlp_item("cat");
    rlp = rlp_push(rlp, rlp_push(rlp_item("cat"), rlp_item("dog")));
    rlp_push(rlp, rlp_item("horse"));
    rlp_push(rlp, rlp_push(NULL, rlp_list()));
    rlp_push(rlp, rlp_item("pig"));
    rlp_push(rlp, rlp_push(NULL, rlp_item("")));
    rlp_push(rlp, rlp_item("sheep"));
    err |= test_item(rlp_random, sizeof(rlp_random), &rlp);

    return err;
}

int
test_u16()
{
    int err = 0;
    uint16_t cat[] = { 'c', 'a', 't' }; //
    uint16_t half[] = { 0x100 };
    uint16_t max[] = { 0xffff };
    uint16_t onefive[] = { 0x000f };
    rlp* rlp;

    rlp = rlp_item_u16_arr(cat, 3);
    err |= test_item(rlp_cat, sizeof(rlp_cat), &rlp);

    rlp = rlp_item_u16_arr(max, 1);
    err |= test_item(rlp_max16, sizeof(rlp_max16), &rlp);

    rlp = rlp_item_u16_arr(half, 1);
    err |= test_item(rlp_half16, sizeof(rlp_half16), &rlp);

    rlp = rlp_item_u16_arr(NULL, 0);
    err |= test_item(rlp_null, sizeof(rlp_null), &rlp);

    rlp = rlp_item_u16_arr(onefive, 1);
    err |= test_item(rlp_15, sizeof(rlp_15), &rlp);

    return err;
}

int
test_u32()
{
    int err = 0;
    uint32_t cat[] = { 'c', 'a', 't' }; //
    uint32_t half[] = { 0x1000000 };
    uint32_t max[] = { 0xffffffff };
    uint32_t onefive[] = { 0x00000f };
    rlp* rlp;

    rlp = rlp_item_u32_arr(cat, 3);
    err |= test_item(rlp_cat, sizeof(rlp_cat), &rlp);

    rlp = rlp_item_u32_arr(max, 1);
    err |= test_item(rlp_max32, sizeof(rlp_max32), &rlp);

    rlp = rlp_item_u32_arr(half, 1);
    err |= test_item(rlp_half32, sizeof(rlp_half32), &rlp);

    rlp = rlp_item_u32_arr(NULL, 0);
    err |= test_item(rlp_null, sizeof(rlp_null), &rlp);

    rlp = rlp_item_u32_arr(onefive, 1);
    err |= test_item(rlp_15, sizeof(rlp_15), &rlp);

    return err;
}

int
test_u64()
{
    int err = 0;
    uint64_t cat[] = { 'c', 'a', 't' }; //
    uint64_t half[] = { 0x100000000000000 };
    uint64_t max[] = { 0xffffffffffffffff };
    uint64_t onefive[] = { 0x0000000f };
    rlp* rlp;

    rlp = rlp_item_u64_arr(cat, 3);
    err |= test_item(rlp_cat, sizeof(rlp_cat), &rlp);

    rlp = rlp_item_u64_arr(max, 1);
    err |= test_item(rlp_max64, sizeof(rlp_max64), &rlp);

    rlp = rlp_item_u64_arr(half, 1);
    err |= test_item(rlp_half64, sizeof(rlp_half64), &rlp);

    rlp = rlp_item_u64_arr(NULL, 0);
    err |= test_item(rlp_null, sizeof(rlp_null), &rlp);

    rlp = rlp_item_u64_arr(onefive, 1);
    err |= test_item(rlp_15, sizeof(rlp_15), &rlp);

    return err;
}

int
test_item(uint8_t* r, uint32_t rlplen, rlp** item_p)
{
    uint8_t result[rlplen];
    uint8_t resulta[rlplen];
    uint32_t len, ret = -1;
    rlp* item = *item_p;
    *item_p = NULL;

    // Check encoded
    len = rlplen;
    rlp_print(item, result, &len);
    if (!(len == rlplen)) goto EXIT;
    if (memcmp(r, result, rlplen)) goto EXIT;
    rlp_free(&item);

    // Check our readback
    item = rlp_parse(result, rlplen);
    rlp_print(item, resulta, &len);
    if (!(len == rlplen)) goto EXIT;
    if (memcmp(r, result, rlplen)) goto EXIT;
    rlp_free(&item);

    memset(result, 0, rlplen);

    // Check decoded
    item = rlp_parse(r, rlplen);
    rlp_print(item, result, &len);
    if (!(len == rlplen)) goto EXIT;
    if (memcmp(r, result, rlplen)) goto EXIT;

    // Test pass
    ret = 0;
EXIT:
    if (item) rlp_free(&item);
    return ret;
}

//
//
//
