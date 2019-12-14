#ifndef HTTP_H
#define HTTP_H

#include "altronix/linq_netw.h"
#include "containers.h"
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
        HTTP_PUT,
        HTTP_DELETE,
    } HTTP_METHOD;

    // Callback for a http request
    typedef void (*http_route_cb)(void*, HTTP_METHOD, uint32_t, const char*);
    typedef struct http_route_context
    {
        http_route_cb cb;
        void* context;
    } http_route_context;
    MAP_INIT_H(routes, http_route_context);

    // Main class context (internal use only)
    typedef struct http_s
    {
        struct mg_connection* listener;
        struct mg_mgr connections;
        routes_map_s* routes;
    } http_s;

    // Public methods (internal to this library)
    void http_init(http_s* http);
    void http_deinit(http_s* http);
    E_LINQ_ERROR http_poll(http_s*, int32_t);
    void http_listen(http_s* http, const char* port);
    void http_use(http_s* http, const char* path, http_route_cb, void*);
    void http_printf_json(void* c, int code, const char* fmt, ...);
    void http_printf(void* c, int code, const char* type, const char* fmt, ...);
    void http_vprintf(void*, int, const char*, uint32_t, const char*, va_list);
#ifdef __cplusplus
}
#endif
#endif
