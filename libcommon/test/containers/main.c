// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "containers.h"
#include "helpers.h"
#include "netw.h"

#include <cmocka.h>
#include <setjmp.h>

typedef struct foo
{
    int foo;
} foo;

typedef struct test
{
    int val;
    void* ctx;
} item_s;

typedef struct car
{
    void* ctx;
} car_s;

void
item_free(item_s** p)
{
    item_s* t = *p;
    *p = NULL;
    linq_network_free(t);
}

void
car_free(car_s** p)
{
    car_s* c = *p;
    *p = NULL;
    linq_network_free(c);
}

// LIST_INIT_H(item, item);
LIST_INIT(item, item_s, item_free);
LIST_INIT(cars, car_s, car_free);

MAP_INIT_H(car, car_s);
MAP_INIT_H(item, item_s);
MAP_INIT(item, item_s, item_free);
MAP_INIT(car, car_s, car_free);
MAP_INIT_W_FREE(foo, foo);

static void
test_container_list_create(void** context_p)
{
    ((void)context_p);
    item_list_s* list = item_list_create();
    item_s* item0 = linq_network_malloc(sizeof(item_s));
    item_s* item1 = linq_network_malloc(sizeof(item_s));
    item_s* item2 = linq_network_malloc(sizeof(item_s));

    item_list_push(list, &item0);
    assert_null(item0);

    item_list_push(list, &item1);
    assert_null(item1);

    item_list_push(list, &item2);
    assert_null(item2);

    assert_int_equal(3, item_list_size(list));

    item0 = item_list_pop(list);

    assert_int_equal(2, item_list_size(list));
    linq_network_free(item0);

    item_list_destroy(&list);
    assert_null(list);
}

static void
test_container_map_create(void** context_p)
{
    ((void)context_p);
    item_map_s* hash = item_map_create();
    item_s* item0 = linq_network_malloc(sizeof(item_s));
    item_s* item1 = linq_network_malloc(sizeof(item_s));
    item_s* item2 = linq_network_malloc(sizeof(item_s));
    item0->val = 0;
    item1->val = 1;
    item2->val = 2;

    assert_non_null(hash);

    item_map_add(hash, "0", &item0);
    assert_null(item0);
    assert_int_equal(1, item_map_size(hash));

    item_map_add(hash, "1", &item1);
    assert_null(item1);
    assert_int_equal(2, item_map_size(hash));

    item_map_add(hash, "2", &item2);
    assert_null(item2);
    assert_int_equal(3, item_map_size(hash));

    item_map_remove(hash, "0");

    assert_int_equal(2, item_map_size(hash));

    item_s** ret = item_map_get(hash, "1");
    assert_int_equal((*ret)->val, 1);

    item_map_destroy(&hash);
    assert_null(hash);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_container_list_create),
        cmocka_unit_test(test_container_map_create)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
