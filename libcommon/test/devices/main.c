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
create_node(
    const char* serial,
    const char* type,
    int transport,
    int birth,
    int last_seen)
{
    node_s* node = malloc(sizeof(node_s));
    memset(node, 0, sizeof(node_s));
    node->free = destroy_node;
    snprintf(node->serial, sizeof(node->serial), "%s", serial);
    snprintf(node->type, sizeof(node->type), "%s", type);
    node->transport = transport;
    node->birth = birth;
    node->last_seen = last_seen;
    return node;
}

static void
jassert_memory_equal(const char* expect, const jsmntok_t* t, const char* alloc)
{
    int l = strlen(expect);
    assert_int_equal(t->end - t->start, l);
    assert_memory_equal(&alloc[t->start], expect, l);
}

static void
jassert_int_equal(uint32_t expect, const jsmntok_t* t, const char* alloc)
{
    char buffer[12];
    uint32_t l = snprintf(buffer, sizeof(buffer), "%d", expect);
    assert_int_equal(t->type, JSMN_PRIMITIVE);
    assert_int_equal(t->end - t->start, l);
    assert_memory_equal(&alloc[t->start], buffer, l);
}

static void
test_device_map_print(void** context_p)
{
    int err;
    char buffer[512];
    node_s* a = create_node("nodea", "linq2", 0, 0, 0);
    node_s* b = create_node("nodeb", "linq8acm", 0, 0, 0);
    node_s* c = create_node("nodec", "linq8pd", 0, 0, 0);

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

static void
test_device_map_foreach(void** context_p)
{
    node_s* a = create_node("nodea", "linqa", TRANSPORT_USB, 1, 10);
    node_s* b = create_node("nodeb", "linqb", TRANSPORT_ZMTP, 2, 20);
    node_s* c = create_node("nodec", "linqc", TRANSPORT_USB, 3, 30);
    device_map_s* devices = device_map_create();
    device_map_add(devices, device_serial(a), &a);
    device_map_add(devices, device_serial(b), &b);
    device_map_add(devices, device_serial(c), &c);

    int count = 0;
    devices_iter iter;
    node_s* node;
    devices_foreach(devices, iter)
    {
        if ((node = devices_iter_exist(devices, iter))) {
            const char* serial = device_serial(node);
            if (!strcmp(serial, "nodea")) {
                assert_int_equal(TRANSPORT_USB, device_transport(node));
                assert_int_equal(1, device_birth(node));
                assert_int_equal(10, device_last_seen(node));
                assert_string_equal("linqa", device_type(node));
                count++;
            } else if (!strcmp(serial, "nodeb")) {
                assert_int_equal(TRANSPORT_ZMTP, device_transport(node));
                assert_int_equal(2, device_birth(node));
                assert_int_equal(20, device_last_seen(node));
                assert_string_equal("linqb", device_type(node));
                count++;
            } else if (!strcmp(serial, "nodec")) {
                assert_int_equal(TRANSPORT_USB, device_transport(node));
                assert_int_equal(3, device_birth(node));
                assert_int_equal(30, device_last_seen(node));
                assert_string_equal("linqc", device_type(node));
                count++;
            }
        }
    }

    assert_int_equal(count, 3);

    device_map_destroy(&devices);
}

static void
test_device_map_alloc_summary(void** context_p)
{
    node_s* nodea = create_node("nodea", "linqa", TRANSPORT_USB, 1, 10);
    node_s* nodeb = create_node("nodeb", "linqb", TRANSPORT_ZMTP, 2, 20);
    node_s* nodec = create_node("nodec", "linqc", TRANSPORT_USB, 3, 30);
    device_map_s* devices = device_map_create();
    device_map_add(devices, device_serial(nodea), &nodea);
    device_map_add(devices, device_serial(nodeb), &nodeb);
    device_map_add(devices, device_serial(nodec), &nodec);
    int err;
    const char* b = devices_summary_alloc(devices);
    assert_non_null(b);
    json_parser p;
    jsontok t[1024];
    json_init(&p);
    err = json_parse(&p, b, strlen(b), t, 1024);
    assert_true(err > 0);

    jassert_memory_equal("nodea", json_delve(b, t, ".nodea.serial"), b);
    jassert_memory_equal("linqa", json_delve(b, t, ".nodea.type"), b);
    jassert_int_equal(TRANSPORT_USB, json_delve(b, t, ".nodea.transport"), b);
    jassert_int_equal(1, json_delve(b, t, ".nodea.birth"), b);
    jassert_int_equal(10, json_delve(b, t, ".nodea.lastSeen"), b);

    jassert_memory_equal("nodeb", json_delve(b, t, ".nodeb.serial"), b);
    jassert_memory_equal("linqb", json_delve(b, t, ".nodeb.type"), b);
    jassert_int_equal(TRANSPORT_ZMTP, json_delve(b, t, ".nodeb.transport"), b);
    jassert_int_equal(2, json_delve(b, t, ".nodeb.birth"), b);
    jassert_int_equal(20, json_delve(b, t, ".nodeb.lastSeen"), b);

    jassert_memory_equal("nodec", json_delve(b, t, ".nodec.serial"), b);
    jassert_memory_equal("linqc", json_delve(b, t, ".nodec.type"), b);
    jassert_int_equal(TRANSPORT_USB, json_delve(b, t, ".nodec.transport"), b);
    jassert_int_equal(3, json_delve(b, t, ".nodec.birth"), b);
    jassert_int_equal(30, json_delve(b, t, ".nodec.lastSeen"), b);

    devices_summary_free(&b);
    device_map_destroy(&devices);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_device_map_print),
        cmocka_unit_test(test_device_map_foreach),
        cmocka_unit_test(test_device_map_alloc_summary)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
