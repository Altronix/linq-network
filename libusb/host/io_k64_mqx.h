#ifndef IO_K64_MQX_H
#define IO_K64_MQX_H

#include "io.h"

#ifndef IO_M5_MAX_OUTGOING
#define IO_M5_MAX_OUTGOING 64
#endif

#ifndef IO_M5_MAX_INCOMING
#define IO_M5_MAX_INCOMING 64
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct io_k64_mqx_s
    {
        io_s io; // Class Base
        uint8_t out[IO_M5_MAX_OUTGOING];
        uint8_t in[IO_M5_MAX_INCOMING];
    } io_k64_mqx_s;

#ifdef __cplusplus
}
#endif
#endif
