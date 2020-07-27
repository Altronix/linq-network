#ifndef ALERT_H_
#define ALERT_H_

#include "database.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int alert_open(database_s*, alert_s*, uint32_t, uint32_t);
    int alert_next(alert_s* a);
    void alert_close(alert_s* a);

#ifdef __cplusplus
}
#endif
#endif

