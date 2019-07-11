#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Dependencies includes
#include "czmq.h"

// project includes
#include "altronix/linq.h"

#define RID_LEN 256
#define SID_LEN 64
#define PID_LEN 64
#define SITE_LEN 64

// Basic packet index's
#define PACKET_MAX 7
#define PACKET_RID_IDX 0
#define PACKET_VER_IDX 1
#define PACKET_TYP_IDX 2
#define PACKET_SID_IDX 3

// Heartbeat IDX
#define PACKET_HB_PID_IDX 4
#define PACKET_HB_SID_IDX 5

// Alert IDX
#define PACKET_ALERT_PID_IDX 4
#define PACKET_ALERT_DAT_IDX 5
#define PACKET_ALERT_DST_IDX 6

// Response IDX
#define PACKET_RESP_ERR_IDX 4
#define PACKET_RESP_DAT_IDX 5

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
