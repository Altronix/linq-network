// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/atx_net.h"
#include "database/database.h"
#include "atx_net_internal.h"
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
        atx_net_free(statement);
    }

    database_row_exists(&d, "devices", "device_id", "test");

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    atx_net_free(statement);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_null(statement);

    database_deinit(&d);
    sqlite_spy_deinit();
}

static void
test_db_insert(void** context_p)
{
    ((void)context_p);

    ((void)context_p);
    sqlite_spy_init();
    outgoing_statement* statement = NULL;
    const char* expect = "INSERT INTO devices("
                         "device_id,"
                         "product,"
                         "prj_version,"
                         "atx_version,"
                         "mfg,"
                         "mac"
                         ") "
                         "VALUES("
                         "serial-id,"
                         "LINQ2,"
                         "2.00.00,"
                         "2.00.00,"
                         "Altronix,"
                         "00:00:00:AA:AA:AA"
                         ");";

    database_s d;
    database_init(&d);

    // Remove any outgoing statements generated from init()
    while ((statement = sqlite_spy_outgoing_statement_pop())) {
        atx_net_free(statement);
    }

    // clang-format off
    database_insert(
        &d,
        "devices",
        6,
        "device_id",   "serial-id",
        "product",     "LINQ2",
        "prj_version", "2.00.00",
        "atx_version", "2.00.00",
        "mfg",         "Altronix",
        "mac",         "00:00:00:AA:AA:AA");
    // clang-format on

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    atx_net_free(statement);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_null(statement);

    database_deinit(&d);
    sqlite_spy_deinit();
}

static void
test_db_insert_n(void** context_p)
{
    ((void)context_p);

    ((void)context_p);
    sqlite_spy_init();
    outgoing_statement* statement = NULL;
    const char* expect = "INSERT INTO devices("
                         "device_id,"
                         "product,"
                         "prj_version,"
                         "atx_version,"
                         "mfg,"
                         "mac"
                         ") "
                         "VALUES("
                         "serial-id,"
                         "LINQ2,"
                         "2.00.00,"
                         "2.00.00,"
                         "Altronix,"
                         "00:00:00:AA:AA:AA"
                         ");";

    database_s d;
    database_init(&d);

    // Remove any outgoing statements generated from init()
    while ((statement = sqlite_spy_outgoing_statement_pop())) {
        atx_net_free(statement);
    }

    // clang-format off
    database_insert_n(
        &d,
        "devices",
        6,
        "device_id",   "serial-id",         strlen("serial-id"),
        "product",     "LINQ2",             strlen("LINQ2"),
        "prj_version", "2.00.00",           strlen("2.00.00"),
        "atx_version", "2.00.00",           strlen("2.00.00"),
        "mfg",         "Altronix",          strlen("Altronix"),
        "mac",         "00:00:00:AA:AA:AA", strlen("00:00:00:AA:AA:AA"));
    // clang-format on

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    atx_net_free(statement);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_null(statement);

    database_deinit(&d);
    sqlite_spy_deinit();
}

static void
test_db_insert_raw(void** context_p)
{

    ((void)context_p);
    sqlite_spy_init();
    outgoing_statement* statement = NULL;
    const char* expect = "INSERT INTO devices("
                         "device_id,"
                         "product,"
                         "prj_version,"
                         "atx_version,"
                         "mfg,"
                         "mac"
                         ") "
                         "VALUES("
                         "serial-id,"
                         "LINQ2,"
                         "2.00.00,"
                         "2.00.00,"
                         "Altronix,"
                         "00:00:00:AA:AA:AA"
                         ");";

    database_s d;
    database_init(&d);

    // Remove any outgoing statements generated from init()
    while ((statement = sqlite_spy_outgoing_statement_pop())) {
        atx_net_free(statement);
    }

    database_insert_raw(
        &d,
        "devices",
        "device_id,product,prj_version,atx_version,mfg,mac",
        "serial-id,LINQ2,2.00.00,2.00.00,Altronix,00:00:00:AA:AA:AA");

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    atx_net_free(statement);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_null(statement);

    database_deinit(&d);
    sqlite_spy_deinit();
}

static void
test_db_insert_raw_n(void** context_p)
{

    ((void)context_p);
    sqlite_spy_init();
    outgoing_statement* statement = NULL;
    char keys[256], vals[256];
    const char* expect = "INSERT INTO devices("
                         "device_id,"
                         "product,"
                         "prj_version,"
                         "atx_version,"
                         "mfg,"
                         "mac"
                         ") "
                         "VALUES("
                         "serial-id,"
                         "LINQ2,"
                         "2.00.00,"
                         "2.00.00,"
                         "Altronix,"
                         "00:00:00:AA:AA:AA"
                         ");";

    database_s d;
    database_init(&d);

    // Remove any outgoing statements generated from init()
    while ((statement = sqlite_spy_outgoing_statement_pop())) {
        atx_net_free(statement);
    }

    uint32_t keys_len = snprintf(
        keys,
        sizeof(keys),
        "%s",
        "device_id,product,prj_version,atx_version,mfg,mac");
    uint32_t vals_len = snprintf(
        vals,
        sizeof(vals),
        "%s",
        "serial-id,LINQ2,2.00.00,2.00.00,Altronix,00:00:00:AA:AA:AA");

    database_insert_raw_n(&d, "devices", keys, keys_len, vals, vals_len);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_string_equal(expect, statement->data);
    atx_net_free(statement);

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
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_db_create),
        cmocka_unit_test(test_db_row_exists),
        cmocka_unit_test(test_db_insert),
        cmocka_unit_test(test_db_insert_n),
        cmocka_unit_test(test_db_insert_raw),
        cmocka_unit_test(test_db_insert_raw_n)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
