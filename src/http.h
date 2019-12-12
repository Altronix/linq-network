#ifndef HTTP_H
#define HTTP_H

#include "altronix/linq_netw.h"
#include "mongoose.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct http_s
    {
        struct mg_connection* listener;
        struct mg_mgr connections;
    } http_s;

    void http_init(http_s* http);
    void http_deinit(http_s* http);
    E_LINQ_ERROR http_poll(http_s*, int32_t);
    void http_listen(http_s* http, const char* port);

#ifdef __cplusplus
}
#endif
#endif
