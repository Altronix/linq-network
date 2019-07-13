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

    request_s* request_create(
        const char* serial,
        const char* path,
        const char* json,
        linq_request_complete_fn fn);
    request_s* request_create_mem(
        const char* serial,
        uint32_t slen,
        const char* path,
        uint32_t plen,
        const char* json,
        uint32_t jlen,
        linq_request_complete_fn fn);
    void request_destroy(request_s** r_p);

    request_list_s* request_list_create();
    void request_list_destroy(request_list_s** list_p);
    void request_list_push(request_list_s* list, request_s** r_p);
    request_s* request_list_pop(request_list_s* list);
    uint32_t request_list_size(request_list_s* list);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_REQUEST_H_ */
