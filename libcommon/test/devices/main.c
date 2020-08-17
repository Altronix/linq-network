// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "libcommon/device.h"

static void
destroy_node(node_s** node_p)
{
    node_s* node = *node_p;
    *node_p = NULL;
    free(node);
}

static node_s*
create_node(const char* serial, const char* type)
{
    node_s* node = malloc(sizeof(node_s));
    memset(node, 0, sizeof(node_s));
    node->free = destroy_node;
    snprintf(node->serial, sizeof(node->serial), "%s", serial);
    snprintf(node->type, sizeof(node->type), "%s", type);
    return node;
}

static void
test_device_map_print(void** context_p)
{
    int err;
    char buffer[512];
    node_s* a = create_node("nodea", "linq2");
    node_s* b = create_node("nodeb", "linq8acm");
    node_s* c = create_node("nodec", "linq8pd");

    device_map_s* devices = device_map_create();
    device_map_add(devices, device_serial(a), &a);
    device_map_add(devices, device_serial(b), &b);
    device_map_add(devices, device_serial(c), &c);

    memset(buffer, 0, sizeof(buffer));
    err = device_map_print(devices, buffer, sizeof(buffer));
    json_parser p;
    jsontok t[128];
    json_init(&p);
    err = json_parse(&p, buffer, err, t, 128);
    assert_true(err > 0);
    json_value proda, prodb, mfg, prod, serial;
    proda = json_delve_value(buffer, t, ".nodea.product");
    prodb = json_delve_value(buffer, t, ".nodeb.product");
    assert_true(proda.p && prodb.p);
    assert_memory_equal(proda.p, "linq2", proda.len);
    assert_memory_equal(prodb.p, "linq8acm", prodb.len);

    device_map_destroy(&devices);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_device_map_print) //
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
