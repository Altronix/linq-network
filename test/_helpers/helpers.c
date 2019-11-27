#include "helpers.h"
#include "linq_netw_internal.h"

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
