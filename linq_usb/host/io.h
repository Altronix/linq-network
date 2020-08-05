#ifndef IO_H
#define IO_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t l;
        uint8_t bytes[];
    } io_packet_s;

    typedef struct io_s
    {
    } io_s;

#ifdef __cplusplus
}
#endif
#endif /* IO_H */
