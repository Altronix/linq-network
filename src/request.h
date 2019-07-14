#ifndef DEVICE_REQUEST_H_
#define DEVICE_REQUEST_H_

#include "czmq.h"
#include "klib/klist.h"

#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct request_s request_s;
    typedef struct request_list_s request_list_s;

    typedef enum E_REQUEST_METHOD
    {
        REQUEST_METHOD_GET,
        REQUEST_METHOD_POST,
        REQUEST_METHOD_DELETE
    } E_REQUEST_METHOD;

    request_s* request_create(
        E_REQUEST_METHOD method,
        const char* serial,
        const char* path,
        const char* json,
        linq_request_complete_fn fn);
    request_s* request_create_mem(
        E_REQUEST_METHOD method,
        const char* serial,
        uint32_t slen,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn);
    void request_destroy(request_s** r_p);
    void request_sent_at(request_s* r, uint32_t at);
    void request_router_id_set(request_s*, uint8_t*, uint32_t);
    const char* request_serial_get(request_s*);
    linq_request_complete_fn request_on_complete_fn(request_s* r);
    int request_send(request_s* r, zsock_t* sock);

    request_list_s* request_list_create();
    void request_list_destroy(request_list_s** list_p);
    void request_list_push(request_list_s* list, request_s** r_p);
    request_s* request_list_pop(request_list_s* list);
    uint32_t request_list_size(request_list_s* list);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_REQUEST_H_ */