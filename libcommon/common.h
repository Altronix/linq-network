#ifndef COMMON_H
#define COMMON_H

#include "sys.h"

#ifndef LINQ_SID_LEN
#define LINQ_SID_LEN 64
#endif

#ifndef LINQ_TID_LEN
#define LINQ_TID_LEN 64
#endif

#ifndef LOG_MESSAGE_LEN
#define LOG_MESSAGE_LEN 128
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        LINQ_TRACE,
        LINQ_DEBUG,
        LINQ_INFO,
        LINQ_WARN,
        LINQ_ERROR,
        LINQ_FATAL,
        LINQ_NONE
    } E_LOG_LEVEL;

    typedef struct log_callback_s
    {
        void* context;
        uint32_t line;
        uint32_t tick;
        const char* file;
        const char* level;
        const char* category;
        char message[LOG_MESSAGE_LEN];
    } log_callback_s;
    typedef void (*log_callback_fn)(log_callback_s*);

#ifdef BUILD_LINQD
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
    typedef struct http_request_s http_request_s;
    typedef void (
        *http_route_cb)(http_request_s*, HTTP_METHOD, uint32_t, const char*);
#endif

    typedef enum E_TRANSPORT
    {
        TRANSPORT_USB,
        TRANSPORT_ZMTP,
    } E_TRANSPORT;

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
        E_TRANSPORT transport;
        void (*poll)(struct node_s*, void*);
        void (*free)(struct node_s**);
        void (*send)(
            struct node_s*,
            E_REQUEST_METHOD method,
            const char* path,
            uint32_t plen,
            const char* json,
            uint32_t jlen,
            linq_request_complete_fn,
            void*);
    } node_s;

    typedef struct netw_s netw_s;
    typedef uint32_t netw_socket;

    typedef struct atx_str
    {
        const char* p;
        uint32_t len;
    } atx_str;

    typedef struct netw_alert_s
    {
        atx_str who;
        atx_str what;
        atx_str where;
        atx_str when;
        atx_str mesg;
        atx_str name;
        atx_str product;
        atx_str email[5];
        char* data;
    } netw_alert_s;

    typedef struct netw_email_s
    {
        atx_str to0;
        atx_str to1;
        atx_str to2;
        atx_str to3;
        atx_str to4;
        atx_str from;
        atx_str subject;
        atx_str user;
        atx_str password;
        atx_str server;
        atx_str port;
        atx_str device;
        char* data;
    } netw_email_s;

    // ERROR
    typedef void (*netw_error_fn)(
        void* context,
        E_LINQ_ERROR error,
        const char*,
        const char*);
    // NEW
    typedef void (*netw_new_fn)(void* context, const char* serial);
    // HEARTBEAT
    typedef void (*netw_heartbeat_fn)(void* context, const char* serial);
    // ALERT
    typedef void (
        *netw_alert_fn)(void*, const char*, netw_alert_s*, netw_email_s*);
    // CTRLC
    typedef void (*netw_ctrlc_fn)(void*);
    typedef void (*netw_devices_foreach_fn)(void*, const char*, const char*);
    typedef struct netw_callbacks
    {
        netw_error_fn on_err;
        netw_new_fn on_new;
        netw_heartbeat_fn on_heartbeat;
        netw_alert_fn on_alert;
        netw_ctrlc_fn on_ctrlc;
    } netw_callbacks;

#ifdef __cplusplus
}
#endif
#endif /* COMMON_H */
