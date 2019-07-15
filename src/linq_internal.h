#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Dependencies includes
#include "czmq.h"

// project includes
#include "altronix/linq.h"

// [router, version, type, serial]
// [router, version, type, serial, product, siteId]      = HEARTBEAT
// [router, version, type, serial, product, alert, mail] = ALERT
// [router, version, type, serial, path [, data] ]       = REQUEST
// [router, version, type, serial, error, data]          = RESPONSE

#define RID_LEN 256
#define SID_LEN 64
#define PID_LEN 64
#define SITE_LEN 64
#define JSON_LEN 1024

// Basic packet index's
#define FRAME_MAX 7
#define FRAME_RID_IDX 0
#define FRAME_VER_IDX 1
#define FRAME_TYP_IDX 2
#define FRAME_SID_IDX 3

// Heartbeat IDX
#define FRAME_HB_PID_IDX 4
#define FRAME_HB_SITE_IDX 5

// Alert IDX
#define FRAME_ALERT_PID_IDX 4
#define FRAME_ALERT_DAT_IDX 5
#define FRAME_ALERT_DST_IDX 6

// Request IDX
#define FRAME_REQ_PATH_IDX 4
#define FRAME_REQ_DATA_IDX 5

// Response IDX
#define FRAME_RES_ERR_IDX 4
#define FRAME_RES_DAT_IDX 5

typedef struct
{
    uint8_t id[256];
    uint32_t sz;
} router_s;

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* LINQ_INTERNAL_H_ */
