#include "altronix/linq.h"
#include "containers.h"
#include "helpers.h"

#include <cmocka.h>
#include <setjmp.h>

typedef struct test
{
    void* ctx;
} item;

typedef struct car
{
    void* ctx;
} car;

void
item_free(item** p)
{
    item* t = *p;
    *p = NULL;
    linq_free(t);
}

void
car_free(car** p)
{
    car* c = *p;
    *p = NULL;
    linq_free(c);
}

LIST_INIT(items, item, item_free);
LIST_INIT(cars, car, car_free);

static void
test_container_list_create(void** context_p)
{
    ((void)context_p);
    list_items_s* list = list_items_create();
    item* item0 = linq_malloc(sizeof(item));
    item* item1 = linq_malloc(sizeof(item));
    item* item2 = linq_malloc(sizeof(item));

    list_items_push(list, &item0);
    assert_null(item0);

    list_items_push(list, &item1);
    assert_null(item1);

    list_items_push(list, &item2);
    assert_null(item2);

    assert_int_equal(3, list_items_size(list));

    item0 = list_items_pop(list);

    assert_int_equal(2, list_items_size(list));
    linq_free(item0);

    list_items_destroy(&list);
    assert_null(list);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_container_list_create)

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
