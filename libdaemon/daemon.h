#ifndef LINQ_DAEMON_H
#define LINQ_DAEMON_H

#include "http.h"
#include "netw.h"
#include "sys.h"

// clang-format off
#if defined _WIN32
#  if defined LINQ_DAEMON_STATIC
#    define LINQ_DAEMON_EXPORT
#  elif defined DLL_EXPORT
#    define LINQ_DAEMON_EXPORT __declspec(dllexport)
#  else
#    define LINQ_DAEMON_EXPORT __declspec(dllimport)
#  endif
#else
#  define LINQ_DAEMON_EXPORT
#endif
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct daemon_config_s
    {
        uint32_t zmtp;
        uint32_t http;
        uint32_t https;
        const char* db_path;
        const char* cert;
        const char* key;
        const char* log;
        bool daemon;
    } daemon_config_s;

    typedef struct daemon_s
    {
        netw_s* netw;
        http_s http;
        bool shutdown;
    } daemon_s;

    LINQ_DAEMON_EXPORT void daemon_init(daemon_s*, daemon_config_s*);
    LINQ_DAEMON_EXPORT void daemon_free(daemon_s*);
    LINQ_DAEMON_EXPORT int daemon_poll(daemon_s*, uint32_t);

#ifdef __cplusplus
}
#endif
#endif
