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

#include "netw.h"
#include "zmq.h"

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
     "\"mesg\": \"Test Alert Message\","                                       \
     "\"name\": \"sysTest\""                                                   \
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

#define DONE 0
#define MORE 1

    typedef struct helpers_test_context_s
    {
        netw_s* net;
    } helpers_test_context_s;

    typedef struct helpers_test_config_s
    {
        netw_callbacks* callbacks;
        void* context;
        uint32_t zmtp;
        uint32_t http;
        const char* user;
        const char* pass;
    } helpers_test_config_s;

    void helpers_test_init();
    void helpers_test_reset();

    helpers_test_context_s* helpers_test_context_create(
        helpers_test_config_s* config);
    void helpers_test_context_destroy(helpers_test_context_s** ctx_p);
    void helpers_add_device(
        helpers_test_context_s* ctx,
        const char* ser,
        const char* rid,
        const char* pid,
        const char* sid);
    void helpers_push_heartbeat(
        const char* router,
        const char* serial,
        const char* product,
        const char* site_id);
    void helpers_push_alert(const char*, const char*, const char*);
    void helpers_push_legacy_alert();
    void helpers_push_response(
        const char* rid,
        const char* sid,
        int16_t err,
        const char* data);
    void helpers_push_request(
        const char* rid,
        const char* sid,
        const char* path,
        const char* data);
    void helpers_push_hello(const char* router, const char* node);
    void helpers_push_incoming_mem(int n, ...);
    void helpers_push_incoming_str(int n, ...);
    void assert_msg_equal(zmq_msg_t* msg, int more, void* data, uint32_t l);
    void assert_recv_msg_equal(int more, void* data, uint32_t l);

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H_ */
