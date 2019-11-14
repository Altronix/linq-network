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
    typedef struct linq_io_s linq_io_s;
    typedef uint32_t linq_io_socket;

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
    } E_LINQ_ERROR;

    typedef struct linq_io_alert_s
    {
        const char* who;
        const char* what;
        const char* where;
        const char* when;
        const char* mesg;
        const char* email[5];
        char* data;
    } linq_io_alert_s;

    typedef struct linq_io_email_s
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
    } linq_io_email_s;

    typedef void (*linq_io_request_complete_fn)(
        void*,
        E_LINQ_ERROR e,
        const char* json,
        device_s**);
    typedef void (*linq_io_error_fn)( //
        void*,
        E_LINQ_ERROR,
        const char*,
        const char*);
    typedef void (*linq_io_heartbeat_fn)( //
        void*,
        const char*,
        device_s**);
    typedef void (*linq_io_alert_fn)( //
        void*,
        linq_io_alert_s*,
        linq_io_email_s*,
        device_s**);
    typedef void (
        *linq_io_devices_foreach_fn)(void* ctx, const char*, const char*);
    typedef struct linq_io_callbacks
    {
        linq_io_error_fn err;
        linq_io_heartbeat_fn hb;
        linq_io_alert_fn alert;
    } linq_io_callbacks;

    // Linq API
    linq_io_s* linq_io_create(const linq_io_callbacks*, void*);
    void linq_io_destroy(linq_io_s**);
    void linq_io_context_set(linq_io_s* linq, void* ctx);
    linq_io_socket linq_io_listen(linq_io_s*, const char* ep);
    linq_io_socket linq_io_connect(linq_io_s* l, const char* ep);
    E_LINQ_ERROR linq_io_shutdown(linq_io_s*, linq_io_socket);
    E_LINQ_ERROR linq_io_disconnect(linq_io_s*, linq_io_socket);
    E_LINQ_ERROR linq_io_poll(linq_io_s* l, uint32_t ms);
    device_s** linq_io_device(const linq_io_s*, const char*);
    uint32_t linq_io_device_count(const linq_io_s*);
    void linq_io_devices_foreach(
        const linq_io_s* l,
        linq_io_devices_foreach_fn,
        void*);
    uint32_t linq_io_nodes_count(const linq_io_s* linq);
    E_LINQ_ERROR linq_io_device_send_get(
        const linq_io_s*,
        const char*,
        const char*,
        linq_io_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_io_device_send_post(
        const linq_io_s*,
        const char*,
        const char*,
        const char*,
        linq_io_request_complete_fn,
        void*);
    E_LINQ_ERROR linq_io_device_send_delete(
        const linq_io_s*,
        const char*,
        const char*,
        linq_io_request_complete_fn,
        void*);

    // Device API
    void device_send_delete(
        device_s*,
        const char*,
        linq_io_request_complete_fn,
        void*);
    void
    device_send_get(device_s*, const char*, linq_io_request_complete_fn, void*);
    void device_send_post(
        device_s*,
        const char*,
        const char*,
        linq_io_request_complete_fn,
        void*);
    void device_send(
        device_s* d,
        const char* path,
        const char* json,
        linq_io_request_complete_fn fn,
        void* context);
    const char* device_serial(device_s* d);

    // Sys API
    bool sys_running();
#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
