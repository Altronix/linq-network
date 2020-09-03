#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "json.h"
#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct config_s
    {
        uint32_t zmtp;
        uint32_t http;
        uint32_t https;
        json_value web_root_path;
        json_value db_path;
        json_value cert;
        json_value key;
        json_value log;
        bool daemon;
    } config_s;

    int parse_config(const char* buff, uint32_t l, config_s* config);
    int print_config(FILE* f, config_s* config);

#ifdef __cplusplus
}
#endif
#endif
