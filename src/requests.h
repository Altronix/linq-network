#ifndef REQUESTS_H_
#define REQUESTS_H_

#include "czmq.h"

#include "linq_internal.h"
#include "request.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct request_list_s request_list_s;

    request_list_s* request_list_create();
    void request_list_destroy(request_list_s** list_p);
    void request_list_push(request_list_s* list, request_s** r_p);
    request_s* request_list_pop(request_list_s* list);
    uint32_t request_list_size(request_list_s* list);

#ifdef __cplusplus
}
#endif

#endif /* REQUESTS_H_ */
