#ifndef LINQ_DAEMON_H
#define LINQ_DAEMON_H

#include "http.h"
#include "linq_network.h"
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

    typedef struct linqd_s
    {
        linq_network_s* netw;
        http_s http;
    } linqd_s;

    void linqd_init(linqd_s*, linqd_config_s*);
    void linqd_free(linqd_s*);
    int linqd_poll(linqd_s*, uint32_t);

#ifdef __cplusplus
}
#endif
#endif
