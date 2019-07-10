#include "helpers.h"

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
