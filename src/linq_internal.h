#ifndef LINQ_INTERNAL_H_
#define LINQ_INTERNAL_H_

// System includes
#include "sys.h"

// Dependencies includes
#include "czmq.h"

// project includes
#include "altronix/linq.h"

// [router, version, type, serial]
// [router, version, 0x00, serial, product, siteId]      = HEARTBEAT
// [router, version, 0x01, serial, path [, data] ]       = REQUEST
// [router, version, 0x02, serial, error, data]          = RESPONSE
// [router, version, 0x03, serial, product, alert, mail] = ALERT
// [router, version, 0x04, serial, ...]                  = HELLO

#define RID_LEN 256
#define SID_LEN 64
#define TID_LEN 64
#define SITE_LEN 64
#define JSON_LEN 1024

#define FRAME_TYP_HEARTBEAT (0x00)
#define FRAME_TYP_REQUEST (0x01)
#define FRAME_TYP_RESPONSE (0x02)
#define FRAME_TYP_ALERT (0x03)
#define FRAME_TYP_HELLO (0x04)

// Basic packet index's
#define FRAME_MAX 7
#define FRAME_RID_IDX 0
#define FRAME_VER_IDX 1
#define FRAME_TYP_IDX 2
#define FRAME_SID_IDX 3

// Heartbeat IDX
#define FRAME_HB_TID_IDX 4
#define FRAME_HB_SITE_IDX 5

// Alert IDX
#define FRAME_ALERT_TID_IDX 4
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
