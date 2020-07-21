// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "linq_daemon.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_utils.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"

static void
test_linqd_receive_heartbeat_insert(void** context_p)
{
    // Init Test Spy
    helpers_test_init();

    // Create LinQD
    linqd_config_s config = { .zmtp = 32820, .http = 8000, .db_path = "./" };
    linqd_s linqd;
    linqd_init(&linqd, &config);

    // Add user
    helpers_test_create_admin(linqd.netw, "unsafe_user", "unsafe_pass");
    helpers_test_context_flush();

    const char* expect_query = "SELECT EXISTS(SELECT 1 FROM devices WHERE "
                               "device_id=\"serial\" LIMIT 1);";
    const char* expect_insert =
        "INSERT INTO "
        "devices(device_id,product,prj_version,atx_version,web_version,mac) "
        "VALUES(\"serial\",\"LINQ2\",\"2.00.00\",\"2.00.00\",\"2.00.00\","
        "\"00:00:00:AA:BB:CC\");";
    const char* response = "{\"about\":{"
                           "\"sid\":\"serial\","
                           "\"product\":\"LINQ2\","
                           "\"prjVersion\":\"2.00.00\","
                           "\"atxVersion\":\"2.00.00\","
                           "\"webVersion\":\"2.00.00\","
                           "\"mac\":\"00:00:00:AA:BB:CC\""
                           "}}";
    zmsg_t* hb0 = helpers_make_heartbeat("rid0", "serial", "product", "site");
    zmsg_t* about = helpers_make_response("rid0", "serial", 0, response);
    outgoing_statement* statement = NULL;

    // Push some incoming heartbeats
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&about);
    czmq_spy_poll_set_incoming((0x01));

    // Database query responses
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(0);

    sqlite_spy_outgoing_statement_flush();

    // Receive a heartbeat (Request about)
    linqd_poll(&linqd, 5);
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_query) + 1, statement->len);
    assert_memory_equal(expect_query, statement->data, statement->len);
    linq_network_free(statement);
    // TODO measure outgoing czmq packet about request

    // Receive about response, and we insert device into database
    linqd_poll(&linqd, 5);
    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_int_equal(strlen(expect_insert) + 1, statement->len);
    assert_memory_equal(expect_insert, statement->data, statement->len);
    linq_network_free(statement);

    linqd_free(&linqd);
    helpers_test_reset();
}

static void
test_linqd_receive_alert_insert(void** context_p)
{
    // Init Test Spy
    helpers_test_init();

    // Create LinQD
    linqd_config_s config = { .zmtp = 32820, .http = 8000, .db_path = "./" };
    linqd_s linqd;
    linqd_init(&linqd, &config);

    // Add user
    helpers_test_create_admin(linqd.netw, "unsafe_user", "unsafe_pass");
    helpers_test_context_flush();

    const char* expect_keys =
        "INSERT INTO "
        "alerts(alert_id,who,what,site_id,time,mesg,name,product,device_id)";
    const char* expect_values =
        "VALUES(\"\",\"TestUser\",\"TestAlert\",\"Altronix Site "
        "ID\",\"1\",\"Test Alert Message\",\"sysTest\",\"\",\"sid\");";
    zmsg_t* hb = helpers_make_heartbeat("rid", "sid", "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", "sid", "pid");
    outgoing_statement* statement;

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    linqd_poll(&linqd, 5);
    sqlite_spy_outgoing_statement_flush();

    linqd_poll(&linqd, 5);

    statement = sqlite_spy_outgoing_statement_pop();
    assert_non_null(statement);
    assert_memory_equal(expect_keys, statement->data, strlen(expect_keys));
    ((void)expect_values); // TODO the uuid is random each test so we can't
                           // compare. (Mocking uuid is challenging)
    linq_network_free(statement);

    // TODO verify websocket broadcast on mongoose outgoing

    linqd_free(&linqd);
    helpers_test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_linqd_receive_heartbeat_insert),
        cmocka_unit_test(test_linqd_receive_alert_insert),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
