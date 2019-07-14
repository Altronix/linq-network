#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <setjmp.h>

#include "cmocka.h"
#include "czmq.h"

#define TEST_ALERT_LEGACY                                                      \
    ("{"                                                                       \
     "\"meth\":\"POST\","                                                      \
     "\"path\":\"home/exe/alert\","                                            \
     "\"post\":{"                                                              \
     "\"who\":\"test-serial-id\","                                             \
     "\"what\":\"Power Supply 1\","                                            \
     "\"siteId\":\"Altronix Site ID\","                                        \
     "\"when\":12345678,"                                                      \
     "\"name\":\"pOn\","                                                       \
     "\"mesg\":\"Power Supply Turn On\""                                       \
     "}"                                                                       \
     "}")

#define TEST_ALERT                                                             \
    ("{"                                                                       \
     "\"who\":\"TestUser\", "                                                  \
     "\"what\": \"TestAlert\","                                                \
     "\"siteId\": \"Altronix Site ID\","                                       \
     "\"when\": 1,"                                                            \
     "\"mesg\": \"Test Alert Message\""                                        \
     "}")

#define TEST_EMAIL                                                             \
    ("{"                                                                       \
     "\"to0\": \"mail0@gmail.com\","                                           \
     "\"to1\": \"mail1@gmail.com\","                                           \
     "\"to2\": \"mail2@gmail.com\","                                           \
     "\"to3\": \"mail3@gmail.com\","                                           \
     "\"to4\": \"mail4@gmail.com\","                                           \
     "\"from\": \"info@altronix.com\","                                        \
     "\"subject\": \"LinQ Alert\","                                            \
     "\"user\": \"\","                                                         \
     "\"password\": \"\","                                                     \
     "\"server\": \"\","                                                       \
     "\"port\": \"\","                                                         \
     "\"device\": \"\""                                                        \
     "}")

#define TEST_ALERT_LEN (sizeof(TEST_ALERT))
#define TEST_EMAIL_LEN (sizeof(TEST_EMAIL))
#define TEST_ALERT_LEGACY_LEN (sizeof(TEST_ALERT_LEGACY))

#ifdef __cplusplus
extern "C"
{
#endif

    zmsg_t* helpers_make_heartbeat(
        const char* router,
        const char* serial,
        const char* product,
        const char* site_id);
    zmsg_t* helpers_make_alert(const char*, const char*, const char*);
    zmsg_t* helpers_make_legacy_alert();
    zmsg_t* helpers_create_message_mem(int n, ...);
    zmsg_t* helpers_create_message_str(int n, ...);

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H_ */
