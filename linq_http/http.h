// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTTP_H
#define HTTP_H

#include "linq_network.h"
#include "containers.h"
#include "database.h"
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
        struct mg_connection* curr_connection;
        struct http_message* curr_message;
        struct http_route_context* self;
    } http_route_context;
    MAP_INIT_H(routes, http_route_context);

    // Main class context (internal use only)
    typedef struct http_s
    {
        struct mg_connection* listener;
        struct mg_mgr connections;
        struct mg_serve_http_opts serve_opts;
        linq_network_s* linq;
        routes_map_s* routes;
        database_s db;
    } http_s;

    // Public methods (internal to this library)
    void http_init(http_s* http, linq_network_s* l);
    void http_deinit(http_s* http);
    E_LINQ_ERROR http_poll(http_s*, int32_t);
    void http_listen(http_s* http, const char* port);
    void http_use(http_s* http, const char* path, http_route_cb, void*);
    void http_serve(http_s* http, const char* path);
    void http_parse_query_str(
        http_route_context* c,
        const char* want,
        const char** result,
        uint32_t* l);
    void http_broadcast_json(http_s* http, int code, const char* fmt, ...);
    void http_printf_json(
        struct mg_connection* connection,
        int code,
        const char* fmt,
        ...);
    void http_printf(
        struct mg_connection* connection,
        int code,
        const char* type,
        const char* fmt,
        ...);
    void http_vprintf(
        struct mg_connection* connection,
        int,
        const char*,
        uint32_t,
        const char*,
        va_list);
#ifdef __cplusplus
}
#endif
#endif
