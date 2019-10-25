#ifndef LINQ_H_
#define LINQ_H_

#include <stdbool.h>
#include <stdint.h>

#define LINQ_ERROR_SOCKET 0xFFFFFFFF

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device_s device_s;
    typedef struct linq_s linq_s;
    typedef uint32_t linq_socket;

    typedef enum
    {
        LINQ_ERROR_OK = 0,
        LINQ_ERROR_OOM = -1,
        LINQ_ERROR_BAD_ARGS = -2,
        LINQ_ERROR_PROTOCOL = -3,
        LINQ_ERROR_IO = -4,
        LINQ_ERROR_DEVICE_NOT_FOUND = -5,
        LINQ_ERROR_TIMEOUT = -6,
    } E_LINQ_ERROR;

    typedef struct linq_alert_s
    {
        const char* who;
        const char* what;
        const char* where;
        const char* when;
        const char* mesg;
        const char* email[5];
        char* data;
    } linq_alert_s;

    typedef struct linq_email_s
    {
        const char* to0;
        const char* to1;
        const char* to2;
        const char* to3;
        const char* to4;
        const char* from;
        const char* subject;
        const char* user;
        const char* password;
        const char* server;
        const char* port;
        const char* device;
        char* data;
    } linq_email_s;

    typedef void (*linq_request_complete_fn)(
        void*,
        E_LINQ_ERROR e,
        const char* json,
        device_s**);
    typedef void (*linq_error_fn)( //
        void*,
        E_LINQ_ERROR,
        const char*,
        const char*);
    typedef void (*linq_heartbeat_fn)( //
        void*,
        const char*,
        device_s**);
    typedef void (*linq_alert_fn)( //
        void*,
        linq_alert_s*,
        linq_email_s*,
        device_s**);
    typedef struct linq_callbacks
    {
        linq_error_fn err;
        linq_heartbeat_fn hb;
        linq_alert_fn alert;
    } linq_callbacks;

    // Linq API
    bool linq_running();
    linq_s* linq_create(const linq_callbacks*, void*);
    void linq_destroy(linq_s**);
    void linq_context_set(linq_s* linq, void* ctx);
    linq_socket linq_listen(linq_s*, const char* ep);
    linq_socket linq_connect(linq_s* l, const char* ep);
    E_LINQ_ERROR linq_shutdown(linq_s*, linq_socket);
    E_LINQ_ERROR linq_disconnect(linq_s*, linq_socket);
    E_LINQ_ERROR linq_poll(linq_s* l, uint32_t ms);
    device_s** linq_device(linq_s*, const char*);
    uint32_t linq_device_count(linq_s*);
    uint32_t linq_nodes_count(linq_s* linq);
    E_LINQ_ERROR linq_device_send_get(
        linq_s*,
        const char*,
        const char*,
        linq_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_device_send_post(
        linq_s*,
        const char*,
        const char*,
        const char*,
        linq_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_device_send_delete(
        linq_s*,
        const char*,
        const char*,
        linq_request_complete_fn,
        void*);

    // Device API
    void
    device_send_delete(device_s*, const char*, linq_request_complete_fn, void*);
    void
    device_send_get(device_s*, const char*, linq_request_complete_fn, void*);
    void device_send_post(
        device_s*,
        const char*,
        const char*,
        linq_request_complete_fn,
        void*);
    void device_send(
        device_s* d,
        const char* path,
        const char* json,
        linq_request_complete_fn fn,
        void* context);
    const char* device_serial(device_s* d);
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
