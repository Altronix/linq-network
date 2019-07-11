#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Dependencies includes
#include "czmq.h"

// project includes
#include "altronix/linq.h"

// Router ID max LEN
#define RID_LEN 256

// Serial ID max LEN
#define SID_LEN 64

// Product ID max LEN
#define PID_LEN 64

// Site ID max LEN
#define SITE_LEN 64

typedef struct
{
    uint8_t id[256];
    uint32_t sz;
} router;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* LINQ_INTERNAL_H_ */
