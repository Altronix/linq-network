#include "config.h"
#define parse_int json_parse_int
#define parse_value json_parse_value
static const char* config_fmt = "{"
                                "\"ports\":{"
                                "\"zmtp\":%d,"
                                "},"
                                "\"nodes\":{"
                                "\"primary\":\"%.*s\","
                                "\"secondary\":\"%.*s\""
                                "},"
                                "\"webRootPath\":\"%.*s\","
                                "\"dbPath\":\"%.*s\","
                                "\"certPath\":\"%.*s\","
                                "\"keyPath\":\"%.*s\","
                                "\"logPath\":\"%.*s\""
                                "}";

void
config_init(config_s* c)
{
    c->daemon = false;
    c->print = false;
    c->zmtp = APP_CONFIG_ZMTP;
    c->save.p = APP_CONFIG_SAVE;
    c->save.len = APP_CONFIG_SAVE_LEN;
    c->node_primary.p = APP_CONFIG_NODE_PRIMARY;
    c->node_primary.len = APP_CONFIG_NODE_PRIMARY_LEN;
    c->node_secondary.p = APP_CONFIG_NODE_SECONDARY;
    c->node_secondary.len = APP_CONFIG_NODE_SECONDARY_LEN;
    c->web_root.p = APP_CONFIG_WEB_ROOT,
    c->web_root.len = APP_CONFIG_WEB_ROOT_LEN;
    c->db.p = APP_CONFIG_DB;
    c->db.len = APP_CONFIG_DB_LEN;
    c->cert.p = APP_CONFIG_CERT;
    c->cert.len = APP_CONFIG_CERT_LEN;
    c->key.p = APP_CONFIG_KEY;
    c->key.len = APP_CONFIG_KEY_LEN;
    c->log.p = APP_CONFIG_LOG;
    c->log.len = APP_CONFIG_LOG_LEN;
}

int
config_parse(const char* buff, uint32_t l, config_s* config)
{
    int err = -1;
    json_parser p;
    jsontok toks[56];
    const jsontok* t;
    json_init(&p);
    err = json_parse(&p, buff, l, toks, 56);
    if (err > 0 && //
        !parse_int(buff, toks, ".ports.zmtp", &config->zmtp) &&
        !parse_value(buff, toks, ".nodes.primary", &config->node_primary) &&
        !parse_value(buff, toks, ".nodes.secondary", &config->node_secondary) &&
        !parse_value(buff, toks, ".webRootPath", &config->web_root) &&
        !parse_value(buff, toks, ".dbPath", &config->db) &&
        !parse_value(buff, toks, ".certPath", &config->cert) &&
        !parse_value(buff, toks, ".keyPath", &config->key) &&
        !parse_value(buff, toks, ".logPath", &config->log)) {
        err = 0;
    }
    return err;
}

int
config_fprint(FILE* f, config_s* config)
{
    // clang-format off
    return fprintf(
        f,
        config_fmt,
        config->zmtp,
        config->node_primary.len,   config->node_primary.p,
        config->node_secondary.len, config->node_secondary.p,
        config->web_root.len,       config->web_root.p,
        config->db.len,             config->db.p,
        config->cert.len,           config->cert.p,
        config->key.len,            config->key.p,
        config->log.len,            config->log.p);
    // clang-format on
}

int
config_print(char* buff, uint32_t l, config_s* config)
{
    // clang-format off
    return snprintf(
        buff,l,
        config_fmt,
        config->zmtp,
        config->node_primary.len,   config->node_primary.p,
        config->node_secondary.len, config->node_secondary.p,
        config->web_root.len,  config->web_root.p,
        config->db.len,        config->db.p,
        config->cert.len,           config->cert.p,
        config->key.len,            config->key.p,
        config->log.len,            config->log.p);
    // clang-format on
}
