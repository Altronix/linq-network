#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "czmq.h"
#include "cmocka.h"

#ifdef __cplusplus
extern "C"
{
#endif

    zmsg_t* helpers_make_legacy_alert();

#ifdef __cplusplus
}
#endif

#endif /* HELPERS_H_ */
