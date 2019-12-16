// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTTP_H
#define HTTP_H

#include "altronix/linq_netw.h"
#include "containers.h"
#include "sys.h"

#include "mongoose.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

#ifdef __cplusplus
extern "C"
{
#endif

    // Type of request
    typedef enum
    {
        HTTP_METHOD_GET,
        HTTP_METHOD_POST,
        HTTP_METHOD_PUT,
        HTTP_METHOD_DELETE,
    } HTTP_METHOD;

    // Callback for a http request
    typedef struct http_route_context http_route_context;
    typedef void (*http_route_cb)(
        http_route_context*,
        HTTP_METHOD,
        uint32_t,
        const char*);
    typedef struct http_route_context
    {
        http_route_cb cb;
        void* context;
        struct mg_connection* connection;
        struct http_message* message;
        struct http_route_context* self;
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
    void
    http_printf_json(http_route_context* c, int code, const char* fmt, ...);
    void http_printf(
        http_route_context* c,
        int code,
        const char* type,
        const char* fmt,
        ...);
    void http_vprintf(
        http_route_context*,
        int,
        const char*,
        uint32_t,
        const char*,
        va_list);
#ifdef __cplusplus
}
#endif
#endif
