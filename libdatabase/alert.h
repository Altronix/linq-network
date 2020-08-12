#ifndef ALERT_H_
#define ALERT_H_

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int alert_open(database_s*, alert_s*, uint32_t, uint32_t);
    void alert_close(alert_s* a);
    int alert_next(alert_s* a);
    int alert_insert(database_s* db, const char* serial, alert_insert_s* a);

#ifdef __cplusplus
}
#endif
#endif

