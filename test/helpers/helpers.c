#include "helpers.h"

zmsg_t*
helpers_make_heartbeat(
    const char* rid,
    const char* sid,
    const char* pid,
    const char* site_id)
{
    zmsg_t* m = helpers_create_message_mem(
        6,
        rid,            // router
        strlen(rid),    //
        "\x0",          // version
        1,              //
        "\x0",          // type
        1,              //
        sid,            // serial
        strlen(sid),    //
        pid,            // product
        strlen(pid),    //
        site_id,        // site id
        strlen(site_id) //
    );
    return m;
}

zmsg_t*
helpers_make_alert(const char* rid, const char* sid, const char* pid)
{
    const char* alert = "{"
                        "\"who\":\"TestUser\", "
                        "\"what\": \"TestAlert\","
                        "\"siteId\": \"Altronix Site ID\","
                        "\"when\": 100,"
                        "\"mesg\": \"Test Alert Message\""
                        "}";
    const char* mail = "{"
                       "\"to0\": \"mail0@gmail.com\","
                       "\"to1\": \"mail1@gmail.com\","
                       "\"to2\": \"mail2@gmail.com\","
                       "\"to3\": \"mail3@gmail.com\","
                       "\"to4\": \"mail4@gmail.com\","
                       "\"from\": \"info@altronix.com\","
                       "\"subject\": \"LinQ Alert\","
                       "\"user\": \"\","
                       "\"password\": \"\","
                       "\"server\": \"\","
                       "\"port\": \"\","
                       "\"device\": \"\""
                       "}";

    zmsg_t* m = helpers_create_message_mem(
        7,
        rid,           // router
        strlen(rid),   //
        "\x0",         // version
        1,             //
        "\x3",         // type
        1,             //
        sid,           // serial
        strlen(sid),   //
        pid,           // product
        strlen(pid),   //
        alert,         // alert
        strlen(alert), //
        mail,          // mail
        strlen(mail)   //
    );
    return m;
}

zmsg_t*
helpers_make_legacy_alert()
{
    const char* alert = "{"
                        "\"meth\":\"POST\","
                        "\"path\":\"home/exe/alert\","
                        "\"post\":{"
                        "\"who\":\"test-serial-id\","
                        "\"what\":\"Power Supply 1\","
                        "\"siteId\":\"Altronix Site ID\","
                        "\"when\":12345678,"
                        "\"name\":\"pOn\","
                        "\"mesg\":\"Power Supply Turn On\""
                        "}"
                        "}";

    return helpers_create_message_str(4, "rid", "sid", "typ", alert);
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
