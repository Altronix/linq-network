// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <setjmp.h>

#include "cmocka.h"

#include "czmq.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

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
     "\"test\":{},"                                                            \
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
    zmsg_t* helpers_make_response(
        const char* rid,
        const char* sid,
        int16_t err,
        const char* data);
    zmsg_t* helpers_make_request(
        const char* rid,
        const char* sid,
        const char* path,
        const char* data);
    zmsg_t* helpers_make_hello(const char* router, const char* node);
    zmsg_t* helpers_create_message_mem(int n, ...);
    zmsg_t* helpers_create_message_str(int n, ...);

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H_ */
