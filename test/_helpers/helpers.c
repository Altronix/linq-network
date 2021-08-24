// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "helpers.h"
#include "mock_zmq.h"
#include "netw_internal.h"
#include "zmtp/zmtp.h"

static void
write_i64(int64_t n, uint8_t* bytes)
{
    bytes[0] = n >> 56;
    bytes[1] = n >> 48;
    bytes[2] = n >> 40;
    bytes[3] = n >> 32;
    bytes[4] = n >> 24;
    bytes[5] = n >> 16;
    bytes[6] = n >> 8;
    bytes[7] = n;
}

static int64_t
read_i64(uint8_t* bytes)
{
    int64_t sz = 0;
    for (int i = 0; i < 8; i++) { ((uint8_t*)&sz)[7 - i] = *bytes++; }
    return sz;
}

void
helpers_test_init()
{
    zmq_spy_init();
}

void
helpers_test_reset()
{
    zmq_spy_free();
}

helpers_test_context_s*
helpers_test_context_create(helpers_test_config_s* config)
{
    char endpoint[64];
    helpers_test_context_s* ctx =
        linq_network_malloc(sizeof(helpers_test_context_s));
    linq_network_assert(ctx);
    memset(ctx, 0, sizeof(helpers_test_context_s));
    ctx->net = netw_create(config->callbacks, config->context);

    if (config->zmtp) {
        snprintf(endpoint, sizeof(endpoint), "tcp://*:%d", config->zmtp);
        netw_listen(ctx->net, endpoint);
    }

    zmq_spy_flush();
    return ctx;
}

void
helpers_test_context_destroy(helpers_test_context_s** ctx_p)
{
    helpers_test_context_s* ctx = *ctx_p;
    *ctx_p = NULL;
    netw_destroy(&ctx->net);
    linq_network_free(ctx);
}

void
helpers_add_device(
    helpers_test_context_s* ctx,
    const char* ser,
    const char* rid,
    const char* pid,
    const char* sid)
{
    // When we receive a heartbeat, we flush out the about request/response
    // that is created by the event
    helpers_push_heartbeat(rid, ser, pid, sid);
    zmq_spy_poll_set_ready((0x01));
    netw_poll(ctx->net, 5);
    zmq_spy_flush();
}

void
helpers_push_heartbeat(
    const char* rid,
    const char* sid,
    const char* pid,
    const char* site_id)
{
    zmq_msg_t msg;
    if (rid) {
        zmq_msg_init_size(&msg, strlen(rid));
        memcpy(zmq_msg_data(&msg), rid, strlen(rid));
        zmq_spy_msg_push_incoming(&msg, ZMQ_SNDMORE);
    }

    helpers_push_mem(
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
}

void
helpers_push_alert(const char* rid, const char* sid, const char* pid)
{
    zmq_msg_t msg;
    if (rid) {
        zmq_msg_init_size(&msg, strlen(rid));
        memcpy(zmq_msg_data(&msg), rid, strlen(rid));
        zmq_spy_msg_push_incoming(&msg, ZMQ_SNDMORE);
    }
    helpers_push_mem(
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
}

void
helpers_push_response(
    const char* rid,
    const char* sid,
    int64_t reqid,
    int16_t err,
    const char* data)
{
    zmq_msg_t msg;
    err = (err >> 8 | err << 8);
    int64_t reqid_packet;
    if (rid) {
        zmq_msg_init_size(&msg, strlen(rid));
        memcpy(zmq_msg_data(&msg), rid, strlen(rid));
        zmq_spy_msg_push_incoming(&msg, ZMQ_SNDMORE);
    }
    write_i64(reqid, (void*)&reqid_packet);
    helpers_push_mem(
        6,
        &g_frame_ver_0,        // version
        1,                     //
        &g_frame_typ_response, // type
        1,                     //
        sid,                   // serial
        strlen(sid),           //
        &reqid_packet,         // reqid
        8,                     //
        &err,                  // error
        2,                     //
        data,                  // data
        strlen(data));         //
}

void
helpers_push_request(
    const char* rid,
    const char* sid,
    const char* path,
    const char* data)
{
    zmq_msg_t msg;
    if (rid) {
        zmq_msg_init_size(&msg, strlen(rid));
        memcpy(zmq_msg_data(&msg), rid, strlen(rid));
        zmq_spy_msg_push_incoming(&msg, ZMQ_SNDMORE);
    }
    data ? helpers_push_mem(
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
         : helpers_push_mem(
               4,
               &g_frame_ver_0,
               1,
               &g_frame_typ_request,
               1,
               sid,
               strlen(sid),
               path,
               strlen(path));
}

void
helpers_push_hello(const char* router, const char* node)
{
    helpers_push_mem(
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

void
helpers_push_str(int n, ...)
{
    va_list list;
    va_start(list, n);
    for (int i = 0; i < n; i++) {
        zmq_msg_t msg;
        char* arg = va_arg(list, char*);
        int sz = strlen(arg);
        zmq_msg_init_size(&msg, strlen(arg) + 1);
        snprintf(zmq_msg_data(&msg), sz + 1, "%s", arg);
        zmq_spy_msg_push_incoming(&msg, i == (n - 1) ? 0 : ZMQ_SNDMORE);
    }
    va_end(list);
}

void
helpers_push_mem(int n, ...)
{
    int err;
    va_list list;
    va_start(list, n);
    for (int i = 0; i < n; i++) {
        zmq_msg_t msg;
        uint8_t* arg = va_arg(list, uint8_t*);
        size_t sz = va_arg(list, size_t);
        err = zmq_msg_init_size(&msg, sz);
        memcpy(zmq_msg_data(&msg), arg, sz);
        zmq_spy_msg_push_incoming(&msg, i == (n - 1) ? 0 : ZMQ_SNDMORE);
    }
    va_end(list);
}

void
assert_msg_equal(zmq_msg_t* msg, int more, void* data, uint32_t l)
{
    void* src = zmq_msg_data(msg);
    assert_int_equal(zmq_msg_size(msg), l);
    assert_int_equal(zmq_msg_more(msg), more);
    assert_memory_equal(src, data, l);
}

void
assert_recv_msg_equal(int more, void* data, uint32_t l)
{
    int sz;
    zmq_msg_t incoming;
    zmq_msg_init(&incoming);
    sz = zmq_msg_recv(&incoming, NULL, 0);
    assert_int_equal(sz, zmq_msg_size(&incoming));
    assert_msg_equal(&incoming, more, data, l);
    zmq_msg_close(&incoming);
}

