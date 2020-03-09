#ifndef LINQ_DAEMON_H
#define LINQ_DAEMON_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct linqd_config_s
    {
        uint16_t zmtp;
        uint16_t http;
        const char* db_path;
    } linqd_config_s;

    void linqd_init(linqd_config_s*);
    void linqd_free(linqd_config_s*);

#ifdef __cplusplus
}
#endif
#endif
