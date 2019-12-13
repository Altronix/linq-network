#ifndef HTTP_H
#define HTTP_H

#include "altronix/linq_netw.h"
#include "mongoose.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Type of request
    typedef enum
    {
        HTTP_GET,
        HTTP_POST,
        HTTP_DELETE,
    } E_HTTP_METHOD;

    typedef struct http_request_s
    {
        void* user_context;
        E_HTTP_METHOD method;
        bool malloced;
        struct
        {
            const char* p;
            uint32_t len;
        } body;
    } http_request_s;

    // Main class context (internal use only)
    typedef struct http_s
    {
        struct mg_connection* listener;
        struct mg_mgr connections;
    } http_s;

    // Public methods (internal to this library)
    void http_init(http_s* http);
    void http_deinit(http_s* http);
    E_LINQ_ERROR http_poll(http_s*, int32_t);
    void http_listen(http_s* http, const char* port);
    void http_printf_json(void* c, int code, const char* fmt, ...);
    void http_printf(void* c, int code, const char* type, const char* fmt, ...);
    void http_vprintf(void*, int, const char*, uint32_t, const char*, va_list);
#ifdef __cplusplus
}
#endif
#endif
