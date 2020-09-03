#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "json.h"
#include "sys.h"

#ifndef LINQD_ZMTP_DEFAULT
#define LINQD_ZMTP_DEFAULT 33247
#endif

#ifndef LINQD_HTTP_DEFAULT
#define LINQD_HTTP_DEFAULT 8080
#endif

#ifndef LINQD_HTTPS_DEFAULT
#define LINQD_HTTPS_DEFAULT 8443
#endif

#ifndef LINQD_WEB_ROOT_PATH_DEFAULT
#define LINQD_WEB_ROOT_PATH_DEFAULT ""
#endif

#ifndef LINQD_DB_PATH_DEFAULT
#define LINQD_DB_PATH_DEFAULT ""
#endif

#ifndef LINQD_CERT_DEFAULT
#define LINQD_CERT_DEFAULT ""
#endif

#ifndef LINQD_KEY_DEFAULT
#define LINQD_KEY_DEFAULT ""
#endif

#ifndef LINQD_LOG_DEFAULT
#define LINQD_LOG_DEFAULT "/var/log/atx-linqd.log"
#endif

#ifndef LINQD_NODE_PRIMARY_DEFAULT
#define LINQD_NODE_PRIMARY_DEFAULT ""
#endif

#ifndef LINQD_NODE_SECONDARY_DEFAULT
#define LINQD_NODE_SECONDARY_DEFAULT ""
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct config_s
    {
        int zmtp;
        int http;
        int https;
        json_value web_root_path;
        json_value db_path;
        json_value cert;
        json_value key;
        json_value log;
        json_value node_primary;
        json_value node_secondary;
        bool daemon;
    } config_s;

    int config_parse(const char* buff, uint32_t l, config_s* config);
    int config_fprint(FILE* f, config_s* config);
    int config_print(char* buff, uint32_t l, config_s* config);

#ifdef __cplusplus
}
#endif
#endif
