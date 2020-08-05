#ifndef IO_M5_H
#define IO_M5_H

#ifndef IO_M5_ENDPOINT_IN
#define IO_M5_ENDPOINT_IN 1
#endif

#ifndef IO_M5_ENDPOINT_OUT
#define IO_M5_ENDPOINT_OUT 2
#endif

#ifndef IO_M5_MAX_OUTGOING
#define IO_M5_MAX_OUTGOING 4096
#endif

#ifndef IO_M5_MAX_INCOMING
#define IO_M5_MAX_INCOMING 4096
#endif

#include "wire.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct io_m5_s
    {
        uint8_t out[IO_M5_MAX_OUTGOING];
        uint8_t in[IO_M5_MAX_INCOMING];
    } io_m5_s;

#ifdef __cplusplus
}
#endif
#endif
