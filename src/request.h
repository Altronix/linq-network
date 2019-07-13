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
    request_list_s* request_list_create();
    void request_list_destroy(request_list_s** list_p);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_REQUEST_H_ */
