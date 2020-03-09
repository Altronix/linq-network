// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "linq_network.h"

#include "helpers.h"
#include "mock_mongoose.h"
#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_alloc(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();

    mongoose_spy_event_request_push("admin:admin", "GET", "/ATX/hello", NULL);
    mongoose_spy_event_request_push(
        "admin:admin", "POST", "/ATX/hello", "{\"hello\":\"world\"}");

    mongoose_spy_deinit();
}

static void
test_mock_push_incoming(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();

    mg_printf(NULL, "a");
    mg_printf(NULL, "b");
    mg_printf(NULL, "c");
    mg_printf(NULL, "d");

    mock_mongoose_outgoing_data* data = mongoose_spy_outgoing_data_pop(4);
    assert_memory_equal(data->mem, "abcd", 4);
    assert_int_equal(data->l, 4);
    mock_mongoose_outgoing_data_destroy(&data);

    mongoose_spy_deinit();
}

static void
test_mock_push_outgoing(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mock_push_incoming),
        cmocka_unit_test(test_mock_push_outgoing)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
