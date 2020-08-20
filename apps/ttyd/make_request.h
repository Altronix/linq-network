#ifndef MAKE_REQUEST_H
#define MAKE_REQUEST_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int make_request(
        const char* meth,
        const char* path,
        const char* data,
        uint32_t l,
        void (*fn)(void*, uint16_t, const char* body),
        void* ctx);

#ifdef __cplusplus
}
#endif
#endif /* MAKE_REQUEST_H */
