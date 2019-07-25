#ifndef DEVICE_REQUEST_H_
#define DEVICE_REQUEST_H_

#include "czmq.h"

#include "linq_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct device_request_s
    {
        router_s forward;
        uint32_t sent_at;
        void* ctx;
        linq_request_complete_fn on_complete;
        zframe_t* frames[FRAME_REQ_DATA_IDX + 1];
    } device_request_s;

    typedef enum E_REQUEST_METHOD
    {
        REQUEST_METHOD_GET,
        REQUEST_METHOD_POST,
        REQUEST_METHOD_DELETE
    } E_REQUEST_METHOD;

    device_request_s* device_request_create(
        E_REQUEST_METHOD method,
        const char* serial,
        const char* path,
        const char* json,
        linq_request_complete_fn fn,
        void*);
    device_request_s* device_request_create_mem(
        E_REQUEST_METHOD method,
        const char* serial,
        uint32_t slen,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn,
        void*);
    device_request_s* device_request_create_from_frames(
        zframe_t* serial,
        zframe_t* path,
        zframe_t* data,
        linq_request_complete_fn,
        void*);
    void device_request_destroy(device_request_s** r_p);
    uint32_t device_request_sent_at(device_request_s* r);
    void device_request_router_id_set(device_request_s*, uint8_t*, uint32_t);
    const char* device_request_serial_get(device_request_s*);
    int device_request_send(device_request_s* r, zsock_t* sock);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_REQUEST_H_ */
