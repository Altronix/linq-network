// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_netw.h"
#include "database/database.h"
#include "linq_netw_internal.h"
#include "mock_sqlite.h"

#include <cmocka.h>
#include <setjmp.h>

static void
test_db_create(void** context_p)
{
    ((void)context_p);
    // TODO test statements when database exists and does not exists
}

static void
test_db_row_exists(void** context_p)
{
    ((void)context_p);
    sqlite_spy_init();
    outgoing_statement* statement = NULL;
    const char* expect = "SELECT EXISTS("
                         "SELECT 1 FROM devices "
                         "WHERE device_id=test "
                         "LIMIT 1);";

    database_s d;
    database_init(&d);

    // Remove any outgoing statements generated from init()
    while ((statement = sqlite_spy_outgoing_statement_pop())) {
        linq_netw_free(statement);
    }

    database_row_exists(&d, "devices", "device_id", "test");

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    linq_netw_free(statement);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_null(statement);

    database_deinit(&d);
    sqlite_spy_deinit();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_db_create),
                                        cmocka_unit_test(test_db_row_exists) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
