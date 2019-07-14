#ifndef REQUESTS_H_
#define REQUESTS_H_

#include "czmq.h"

#include "linq_internal.h"
#include "request.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct requests_s requests_s;

    requests_s* requests_create();
    void requests_destroy(requests_s** list_p);
    void requests_push(requests_s* list, request_s** r_p);
    request_s* requests_pop(requests_s* list);
    uint32_t requests_size(requests_s* list);

#ifdef __cplusplus
}
#endif

#endif /* REQUESTS_H_ */
