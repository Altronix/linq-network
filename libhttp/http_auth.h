// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTTP_AUTH_H
#define HTTP_AUTH_H

#include "database.h"
#include "jsmn/jsmn_tokens.h"
#include "sys.h"

#define _WINSOCKAPI_
#include "mongoose.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

#define UUID_MAX_LEN 32 + 1
#define USER_MAX_LEN 24
#define PASS_MAX_LEN 48
#define PASS_MIN_LEN 12
#define SALT_LEN 16
#define HASH_LEN 64
#define SECRET_LEN 64

#define HTTP_AUTH_TOKEN_FMT "{\"sub\":\"%s\",\"iat\":%d,\"exp\":%d}"

#ifdef __cplusplus
extern "C"
{
#endif

    bool http_auth_is_authorized(
        database_s* db,
        struct mg_connection* c,
        struct http_message* m);

    int http_auth_generate_password_hash(
        char hash[HASH_LEN],
        char salt[SALT_LEN],
        char* pass,
        uint32_t len);

    void http_auth_generate_uuid(char uuid[UUID_MAX_LEN]);

    int http_auth_login(
        database_s* database,
        const char* user,
        const char* password,
        jsmn_token_encode_s* token);

#ifdef __cplusplus
}
#endif
#endif
