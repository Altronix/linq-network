// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "atx_net_internal.h"
#include "database/database.h"

#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"

void
helpers_test_init(const char* user, const char* password)
{
    ((void)user);
    ((void)password);
    mongoose_spy_init();
    sqlite_spy_init();
    sqlite_spy_step_return_push(SQLITE_DONE); // PRAGMA

    for (int i = 0; i < NUM_DATABASES; i++) {
        sqlite_spy_step_return_push(SQLITE_ROW);
    }
}

void
helpers_test_reset()
{
    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

void
helpers_test_create_admin(
    helpers_test_context_s* test,
    const char* user,
    const char* pass)
{
    const char* req_path = "/api/v1/linq-lite/create_admin";
    char b[128];
    int l;
    l = snprintf(b, sizeof(b), "{\"user\":\"%s\",\"pass\":\"%s\"}", user, pass);

    mongoose_spy_event_request_push("", "POST", req_path, b);
    for (int i = 0; i < 4; i++) atx_net_poll(test->net, -1);
}

helpers_test_context_s*
helpers_test_context_create(helpers_test_config_s* config)
{
    char endpoint[64];
    helpers_test_context_s* ctx =
        atx_net_malloc(sizeof(helpers_test_context_s));
    atx_net_assert(ctx);
    helpers_test_init(config->user, config->pass);
    ctx->net = atx_net_create(config->callbacks, config->context);
    sqlite_spy_outgoing_statement_flush();

    if (config->zmtp) {
        snprintf(endpoint, sizeof(endpoint), "tcp://*:%d", config->zmtp);
        atx_net_listen(ctx->net, endpoint);
    }

    if (config->http) {
        snprintf(endpoint, sizeof(endpoint), "http://*:%d", config->zmtp);
        atx_net_listen(ctx->net, endpoint);
    }
    return ctx;
}

void
helpers_test_context_destroy(helpers_test_context_s** ctx_p)
{
    helpers_test_context_s* ctx = *ctx_p;
    *ctx_p = NULL;
    atx_net_destroy(&ctx->net);
    atx_net_free(ctx);
    helpers_test_reset();
}

zmsg_t*
helpers_make_heartbeat(
    const char* rid,
    const char* sid,
    const char* pid,
    const char* site_id)
{
    zmsg_t* m = helpers_create_message_mem(
        5,
        &g_frame_ver_0,         // version
        1,                      //
        &g_frame_typ_heartbeat, // type
        1,                      //
        sid,                    // serial
        strlen(sid),            //
        pid,                    // product
        strlen(pid),            //
        site_id,                // site id
        strlen(site_id)         //
    );
    if (rid) {
        zframe_t* r = zframe_new(rid, strlen(rid));
        zmsg_prepend(m, &r);
    }
    return m;
}

zmsg_t*
helpers_make_alert(const char* rid, const char* sid, const char* pid)
{
    zmsg_t* m = helpers_create_message_mem(
        6,
        &g_frame_ver_0,     // version
        1,                  //
        &g_frame_typ_alert, // type
        1,                  //
        sid,                // serial
        strlen(sid),        //
        pid,                // product
        strlen(pid),        //
        TEST_ALERT,         // alert
        strlen(TEST_ALERT), //
        TEST_EMAIL,         // mail
        strlen(TEST_EMAIL)  //
    );
    if (rid) {
        zframe_t* r = zframe_new(rid, strlen(rid));
        zmsg_prepend(m, &r);
    }
    return m;
}

zmsg_t*
helpers_make_legacy_alert()
{
    return helpers_create_message_str(
        4, "rid", "sid", "typ", TEST_ALERT_LEGACY);
}

zmsg_t*
helpers_make_response(
    const char* rid,
    const char* sid,
    int16_t err,
    const char* data)
{
    err = (err >> 8 | err << 8);
    zmsg_t* m = helpers_create_message_mem(
        5,
        &g_frame_ver_0,        // version
        1,                     //
        &g_frame_typ_response, // type
        1,                     //
        sid,                   // serial
        strlen(sid),           //
        &err,                  // error
        2,                     //
        data,                  // data
        strlen(data));         //
    if (rid) {
        zframe_t* r = zframe_new(rid, strlen(rid));
        zmsg_prepend(m, &r);
    }
    return m;
}

zmsg_t*
helpers_make_request(
    const char* rid,
    const char* sid,
    const char* path,
    const char* data)
{
    zmsg_t* m = data ? helpers_create_message_mem(
                           5,
                           &g_frame_ver_0,
                           1,
                           &g_frame_typ_request,
                           1,
                           sid,
                           strlen(sid),
                           path,
                           strlen(path),
                           data,
                           strlen(data))
                     : helpers_create_message_mem(
                           4,
                           &g_frame_ver_0,
                           1,
                           &g_frame_typ_request,
                           1,
                           sid,
                           strlen(sid),
                           path,
                           strlen(path));
    if (rid) {
        zframe_t* r = zframe_new(rid, strlen(rid));
        zmsg_prepend(m, &r);
    }
    return m;
}

zmsg_t*
helpers_make_hello(const char* router, const char* node)
{
    return helpers_create_message_mem(
        4,
        router,
        strlen(router),
        &g_frame_ver_0,
        1,
        &g_frame_typ_hello,
        1,
        node,
        strlen(node));
}

zmsg_t*
helpers_create_message_str(int n, ...)
{
    va_list list;
    va_start(list, n);
    zmsg_t* msg = zmsg_new();
    assert_non_null(msg);
    for (int i = 0; i < n; i++) {
        char* arg = va_arg(list, char*);
        zframe_t* frame = zframe_new(arg, strlen(arg));
        assert_non_null(frame);
        zmsg_append(msg, &frame);
    }
    va_end(list);
    return msg;
}

// TODO set more flag on frames...
zmsg_t*
helpers_create_message_mem(int n, ...)
{
    va_list list;
    va_start(list, n);
    zmsg_t* msg = zmsg_new();
    assert_non_null(msg);
    for (int i = 0; i < n; i++) {
        uint8_t* arg = va_arg(list, uint8_t*);
        size_t sz = va_arg(list, size_t);
        zframe_t* frame = zframe_new(arg, sz);
        assert_non_null(frame);
        zmsg_append(msg, &frame);
    }
    va_end(list);
    return msg;
}
