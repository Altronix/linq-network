#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <setjmp.h>

#include "cmocka.h"
#include "czmq.h"

#ifdef __cplusplus
extern "C"
{
#endif

    zmsg_t* helpers_make_heartbeat(
        const char* router,
        const char* serial,
        const char* product,
        const char* site_id);
    zmsg_t* helpers_make_legacy_alert();
    zmsg_t* helpers_create_message_mem(int n, ...);
    zmsg_t* helpers_create_message_str(int n, ...);

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H_ */
