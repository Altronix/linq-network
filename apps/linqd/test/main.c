#include "json.h"
#include "mock_file.h"
#include "sys.h"

#include "../config.h"

#include <setjmp.h>

#include "cmocka.h"

static const char* config = "{"
                            "\"ports\":{"
                            "\"zmtp\":100,"
                            "\"http\":200,"
                            "\"https\":333"
                            "},"
                            "\"nodes\":{"
                            "\"primary\":\"tcp://primary\","
                            "\"secondary\":\"tcp://secondary\""
                            "},"
                            "\"webRootPath\":\"webRootPathValue\","
                            "\"dbPath\":\"dbPathValue\","
                            "\"certPath\":\"certValue\","
                            "\"keyPath\":\"keyValue\","
                            "\"logPath\":\"logValue\""
                            "}";
static void
test_config_parse(void** context_p)
{

    config_s c;
    int rc = config_parse(config, strlen(config), &c);
    assert_int_equal(rc, 0);
    assert_int_equal(c.zmtp, 100);
    assert_int_equal(c.http, 200);
    assert_int_equal(c.https, 333);
    assert_int_equal(c.node_primary.len, 13);
    assert_memory_equal(c.node_primary.p, "tcp://primary", 13);
    assert_int_equal(c.node_secondary.len, 15);
    assert_memory_equal(c.node_secondary.p, "tcp://secondary", 15);
    assert_int_equal(c.web_root.len, 16);
    assert_memory_equal(c.web_root.p, "webRootPathValue", 16);
    assert_int_equal(c.db.len, 11);
    assert_memory_equal(c.db.p, "dbPathValue", 11);
    assert_int_equal(c.cert.len, 9);
    assert_memory_equal(c.cert.p, "certValue", 9);
    assert_int_equal(c.key.len, 8);
    assert_memory_equal(c.key.p, "keyValue", 8);
    assert_int_equal(c.log.len, 8);
    assert_memory_equal(c.log.p, "logValue", 8);
}

static void
test_config_fprint(void** context_p)
{
    config_s c = {
        .zmtp = 100,
        .http = 200,
        .https = 333,
        .node_primary = { .p = "tcp://primary", .len = 13 },
        .node_secondary = { .p = "tcp://secondary", .len = 15 },
        .web_root = { .p = "webRootPathValue", .len = 16 },
        .db = { .p = "dbPathValue", .len = 11 },
        .cert = { .p = "certValue", .len = 9 },
        .key = { .p = "keyValue", .len = 8 },
        .log = { .p = "logValue", .len = 8 },
    };
    spy_file_packet_s* pack;

    spy_file_init();

    config_fprint(NULL, &c);
    pack = spy_file_packet_pop_outgoing();
    assert_non_null(pack);
    assert_int_equal(pack->len, strlen(config));
    assert_memory_equal(pack->bytes, config, pack->len);
    spy_file_packet_free(&pack);

    spy_file_free();
}

static void
test_config_print(void** context_p)
{
    config_s c = {
        .zmtp = 100,
        .http = 200,
        .https = 333,
        .node_primary = { .p = "tcp://primary", .len = 13 },
        .node_secondary = { .p = "tcp://secondary", .len = 15 },
        .web_root = { .p = "webRootPathValue", .len = 16 },
        .db = { .p = "dbPathValue", .len = 11 },
        .cert = { .p = "certValue", .len = 9 },
        .key = { .p = "keyValue", .len = 8 },
        .log = { .p = "logValue", .len = 8 },
    };
    char buffer[512];

    config_print(buffer, sizeof(buffer), &c);
    assert_int_equal(strlen(buffer), strlen(config));
    assert_memory_equal(buffer, config, strlen(buffer));
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_config_parse),
        cmocka_unit_test(test_config_fprint),
        cmocka_unit_test(test_config_print),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
