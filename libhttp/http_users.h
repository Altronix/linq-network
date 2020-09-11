#ifndef HTTP_USERS_H
#define HTTP_USERS_H

#include "http.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void has_admin(http_request_s*, HTTP_METHOD, uint32_t, const char*);
    void create_admin(http_request_s*, HTTP_METHOD, uint32_t, const char*);
    void login(http_request_s*, HTTP_METHOD, uint32_t, const char*);
    void users(http_request_s*, HTTP_METHOD, uint32_t, const char*);

#ifdef __cplusplus
}
#endif
#endif /* HTTP_USERS_H */
