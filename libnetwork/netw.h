// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_H_
#define LINQ_H_

#include "sys.h"

#include "common.h"

// clang-format off
#if defined _WIN32
#  if defined LINQ_STATIC
#    define LINQ_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_EXPORT __declspec(dllexport)
#  else
#    define LINQ_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_EXPORT
#endif
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    // Linq API
    LINQ_EXPORT const char* netw_version();
    LINQ_EXPORT netw_s* netw_create(const netw_callbacks*, void*);
    LINQ_EXPORT void netw_destroy(netw_s**);
    LINQ_EXPORT void netw_init(netw_s*, const netw_callbacks*, void*);
    LINQ_EXPORT void netw_deinit(netw_s*);
    LINQ_EXPORT void netw_context_set(netw_s* linq, void* ctx);
    LINQ_EXPORT void netw_root(netw_s* linq, const char* root);
    LINQ_EXPORT netw_socket netw_listen(netw_s*, const char* ep);
    LINQ_EXPORT netw_socket netw_connect(netw_s* l, const char* ep);
    LINQ_EXPORT E_LINQ_ERROR netw_close(netw_s*, netw_socket);
    LINQ_EXPORT E_LINQ_ERROR netw_poll(netw_s* l, int32_t ms);
    LINQ_EXPORT node_s** netw_device(const netw_s* l, const char* serial);
    LINQ_EXPORT bool netw_device_exists(const netw_s*, const char* sid);
    LINQ_EXPORT uint32_t netw_device_count(const netw_s*);
    LINQ_EXPORT void
    netw_devices_foreach(const netw_s* l, netw_devices_foreach_fn, void*);
    LINQ_EXPORT uint32_t netw_node_count(const netw_s* linq);
    LINQ_EXPORT E_LINQ_ERROR netw_send(
        const netw_s* linq,
        const char* sid,
        const char* meth,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn,
        void* ctx);
    LINQ_EXPORT bool netw_running(netw_s* netw);

    LINQ_EXPORT int netw_scan(netw_s*);
    LINQ_EXPORT void netw_shutdown(netw_s* netw);

#ifdef BUILD_LINQD
    typedef struct database_s database_s;
    LINQ_EXPORT database_s* netw_database(netw_s* l);
    LINQ_EXPORT void
    netw_use(netw_s* netw, const char* path, http_route_cb cb, void* context);
#endif
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
