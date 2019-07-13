#ifndef LINQ_H_
#define LINQ_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device device;
    typedef struct devices devices;
    typedef struct linq linq;

    typedef enum
    {
        LINQ_ERROR_OK = 0,
        LINQ_ERROR_OOM = -1,
        LINQ_ERROR_BAD_ARGS = -2,
        LINQ_ERROR_PROTOCOL = -3
    } E_LINQ_ERROR;

    typedef struct linq_alert
    {
        const char* who;
        const char* what;
        const char* where;
        const char* when;
        const char* mesg;
        const char* email[5];
    } linq_alert;

    typedef struct linq_email
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
    } linq_email;

    typedef void (
        *linq_error_fn)(void*, E_LINQ_ERROR, const char*, const char*);
    typedef void (*linq_heartbeat_fn)(void*, const char*, device**);
    typedef void (*linq_alert_fn)(void*, linq_alert*, linq_email*, device**);
    typedef struct linq_callbacks
    {
        linq_error_fn err;
        linq_heartbeat_fn hb;
        linq_alert_fn alert;
    } linq_callbacks;

    linq* linq_create(linq_callbacks*, void*);
    void linq_destroy(linq**);
    E_LINQ_ERROR linq_listen(linq*, const char* ep);
    E_LINQ_ERROR linq_poll(linq* l);
    device** linq_device(linq*, const char*);
    uint32_t linq_device_count(linq*);

#ifdef __cplusplus
}
#endif
#endif /* LINQ_H_ */
