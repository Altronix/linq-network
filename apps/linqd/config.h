#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "json.h"
#include "stdio.h"

#ifndef APP_CONFIG_ZMTP
#define APP_CONFIG_ZMTP 33247
#endif

#ifndef APP_CONFIG_HTTP
#define APP_CONFIG_HTTP 8080
#endif

#ifndef APP_CONFIG_HTTPS
#define APP_CONFIG_HTTPS 8443
#endif

#ifndef APP_CONFIG_SAVE
#define APP_CONFIG_SAVE NULL
#endif
#define APP_CONFIG_SAVE_LEN (APP_CONFIG_SAVE ? sizeof(APP_CONFIG_SAVE) - 1 : 0)

#ifndef APP_CONFIG_WEB_ROOT
#define APP_CONFIG_WEB_ROOT NULL
#endif
#define APP_CONFIG_WEB_ROOT_LEN                                                \
    (APP_CONFIG_WEB_ROOT ? sizeof(APP_CONFIG_WEB_ROOT) - 1 : 0)

#ifndef APP_CONFIG_DB
#define APP_CONFIG_DB NULL
#endif
#define APP_CONFIG_DB_LEN (APP_CONFIG_DB ? sizeof(APP_CONFIG_DB) - 1 : 0)

#ifndef APP_CONFIG_CERT
#define APP_CONFIG_CERT NULL
#endif
#define APP_CONFIG_CERT_LEN                                                    \
    (APP_CONFIG_CERT ? (sizeof(APP_CONFIG_CERT) - 1) : 0)

#ifndef APP_CONFIG_KEY
#define APP_CONFIG_KEY NULL
#endif
#define APP_CONFIG_KEY_LEN (APP_CONFIG_KEY ? sizeof(APP_CONFIG_KEY) - 1 : 0)

#ifndef APP_CONFIG_LOG
#define APP_CONFIG_LOG "/var/log/atx-linqd.log"
#endif
#define APP_CONFIG_LOG_LEN (APP_CONFIG_LOG ? sizeof(APP_CONFIG_LOG) - 1 : 0)

#ifndef APP_CONFIG_NODE_PRIMARY
#define APP_CONFIG_NODE_PRIMARY NULL
#endif
#define APP_CONFIG_NODE_PRIMARY_LEN                                            \
    (APP_CONFIG_NODE_PRIMARY ? sizeof(APP_CONFIG_NODE_PRIMARY) - 1 : 0)

#ifndef APP_CONFIG_NODE_SECONDARY
#define APP_CONFIG_NODE_SECONDARY NULL
#endif
#define APP_CONFIG_NODE_SECONDARY_LEN                                          \
    (APP_CONFIG_NODE_SECONDARY ? sizeof(APP_CONFIG_NODE_SECONDARY) - 1 : 0)

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct config_s
    {
        bool daemon;
        bool print;
        int zmtp;
        json_value save;
        json_value web_root;
        json_value db;
        json_value cert;
        json_value key;
        json_value log;
        json_value node_primary;
        json_value node_secondary;
    } config_s;

    void config_init(config_s* config);
    int config_parse(const char*, uint32_t l, config_s*);
    int config_fprint(FILE* f, config_s* config);
    int config_print(char* buff, uint32_t l, config_s* config);

#ifdef __cplusplus
}
#endif
#endif
