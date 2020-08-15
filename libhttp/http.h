// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HTTP_H
#define HTTP_H

#include "common.h"
#include "containers.h"
#include "database.h"
#include "sys.h"

#include "mongoose.h"
#undef closesocket    // Mongoose and czmq both define these
#undef INVALID_SOCKET // Mongoose and czmq both define these

// clang-format off
#if defined _WIN32
#  if defined LINQ_HTTP_STATIC
#    define LINQ_HTTP_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_HTTP_EXPORT __declspec(dllexport)
#  else
#    define LINQ_HTTP_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_HTTP_EXPORT
#endif
// clang-format on

#define JERROR_200 "{\"error\":\"Ok\"}"
#define JERROR_400 "{\"error\":\"Bad request\"}"
#define JERROR_404 "{\"error\":\"Not found\"}"
#define JERROR_500 "{\"error\":\"Server error\"}"
#define JERROR_503 "{\"error\":\"Unauthorized\"}"
#define JERROR_504 "{\"error\":\"Server busy\"}"

#ifdef __cplusplus
extern "C"
{
#endif

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
        struct mg_connection* http;
        struct mg_connection* https;
        struct mg_mgr connections;
        struct mg_serve_http_opts serve_opts;
        routes_map_s* routes;
        database_s* db;
    } http_s;

    // Public methods (internal to this library)
    LINQ_HTTP_EXPORT void http_init(http_s* http, database_s*);
    LINQ_HTTP_EXPORT void http_deinit(http_s* http);
    LINQ_HTTP_EXPORT E_LINQ_ERROR http_poll(http_s*, int32_t);
    LINQ_HTTP_EXPORT void http_listen(http_s* http, const char* port);
    LINQ_HTTP_EXPORT void
    http_listen_tls(http_s*, const char*, const char*, const char*);
    LINQ_HTTP_EXPORT void http_use(http_s*, const char*, http_route_cb, void*);
    LINQ_HTTP_EXPORT void http_serve(http_s* http, const char* path);
    LINQ_HTTP_EXPORT void http_parse_query_str(
        http_route_context* c,
        const char* want,
        const char** result,
        uint32_t* l);
    LINQ_HTTP_EXPORT void
    http_broadcast_json(http_s* http, int code, const char* fmt, ...);
    LINQ_HTTP_EXPORT void http_printf_json(
        struct mg_connection* connection,
        int code,
        const char* fmt,
        ...);
    LINQ_HTTP_EXPORT void http_printf(
        struct mg_connection* connection,
        int code,
        const char* type,
        const char* fmt,
        ...);
    LINQ_HTTP_EXPORT void http_vprintf(
        struct mg_connection* connection,
        int,
        const char*,
        uint32_t,
        const char*,
        va_list);
    LINQ_HTTP_EXPORT const char* http_error_message(E_LINQ_ERROR);
    LINQ_HTTP_EXPORT uint32_t http_error_code(E_LINQ_ERROR);
#ifdef __cplusplus
}
#endif
#endif
