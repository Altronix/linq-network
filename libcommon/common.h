#ifndef COMMON_H
#define COMMON_H

#include "sys.h"

#ifndef LINQ_SID_LEN
#define LINQ_SID_LEN 64
#endif

#ifndef LINQ_TID_LEN
#define LINQ_TID_LEN 64
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum E_REQUEST_METHOD
    {
        REQUEST_METHOD_RAW = 0,
        REQUEST_METHOD_GET = 1,
        REQUEST_METHOD_POST = 2,
        REQUEST_METHOD_DELETE = 3
    } E_REQUEST_METHOD;

    typedef enum
    {
        LINQ_ERROR_OK = 0,
        LINQ_ERROR_OOM = -1,
        LINQ_ERROR_BAD_ARGS = -2,
        LINQ_ERROR_PROTOCOL = -3,
        LINQ_ERROR_IO = -4,
        LINQ_ERROR_DEVICE_NOT_FOUND = -5,
        LINQ_ERROR_TIMEOUT = -6,
        LINQ_ERROR_SHUTTING_DOWN = -7,
        LINQ_ERROR_400 = 400,
        LINQ_ERROR_403 = 403,
        LINQ_ERROR_404 = 404,
        LINQ_ERROR_500 = 500,
        LINQ_ERROR_504 = 504,
    } E_LINQ_ERROR;

    typedef void (*linq_request_complete_fn)(
        void*,
        const char* serial,
        E_LINQ_ERROR e,
        const char* json);

    typedef struct request_s
    {
        linq_request_complete_fn callback;
        uint32_t sent_at;
        uint32_t retry_at;
        uint32_t retry_count;
        void* ctx;
    } request_s;

    typedef struct node_s
    {
        request_s* pending;
        char serial[LINQ_SID_LEN];
        char type[LINQ_TID_LEN];
        uint32_t birth;
        uint32_t uptime;
        uint32_t last_seen;
    } node_s;

#ifdef __cplusplus
}
#endif
#endif /* COMMON_H */
