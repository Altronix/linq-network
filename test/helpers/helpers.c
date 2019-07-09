#include "helpers.h"

zmsg_t*
helpers_make_legacy_alert()
{
    zmsg_t* msg = zmsg_new();
    zframe_t *rid, *typ, *sid, *dat;
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
    rid = zframe_new("sid", 3);
    sid = zframe_new("rid", 3);
    typ = zframe_new("typ", 3);
    dat = zframe_new(alert, strlen(alert) + 1);

    assert_non_null(msg);
    assert_true(rid && sid && typ && dat);
    zmsg_append(msg, &rid);
    zmsg_append(msg, &typ);
    zmsg_append(msg, &sid);
    zmsg_append(msg, &dat);
    return msg;
}

