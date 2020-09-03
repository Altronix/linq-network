#include "config.h"
static const char* config_fmt = "{"
                                "\t\"ports\":{"
                                "\t\t\"zmtp\":%d,"
                                "\t\t\"http\":%d,"
                                "\t\t\"https\":%d"
                                "\t},"
                                "\t\"nodes\":{"
                                "\t\t\"primary\":\"%.*s\","
                                "\t\t\"secondary\":\"%.*s\""
                                "\t},"
                                "\t\"webRootPath\":\"%.*s\","
                                "\t\"dbPath\":\"%.*s\","
                                "\t\"certPath\":\"%.*s\","
                                "\t\"keyPath\":\"%.*s\","
                                "\t\"logPath\":\"%.*s\""
                                "}";

// TODO move to json
static int
parse_int(const char* buff, jsontok* toks, const char* guide, int* result)
{
    char b[12];
    const jsontok* t = json_delve(buff, toks, guide);
    if (t) {
        snprintf(b, sizeof(b), "%.*s", t->end - t->start, &buff[t->start]);
        *result = atoi(b);
        return 0;
    } else {
        return -1;
    }
}

// TODO move to json
static int
parse_value(const char* buff, jsontok* toks, const char* guide, json_value* v)
{
    const jsontok* t = json_delve(buff, toks, guide);
    if (t) {
        *v = json_tok_value(buff, t);
        return 0;
    } else {
        return -1;
    }
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
        !parse_int(buff, toks, ".ports.http", &config->http) &&
        !parse_int(buff, toks, ".ports.https", &config->https) &&
        !parse_value(buff, toks, ".nodes.primary", &config->node_primary) &&
        !parse_value(buff, toks, ".nodes.secondary", &config->node_secondary) &&
        !parse_value(buff, toks, ".webRootPath", &config->web_root_path) &&
        !parse_value(buff, toks, ".dbPath", &config->db_path) &&
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
        config->http,
        config->https,
        config->node_primary.len,   config->node_primary.p,
        config->node_secondary.len, config->node_secondary.p,
        config->web_root_path.len,  config->web_root_path.p,
        config->db_path.len,        config->db_path.p,
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
        config->http,
        config->https,
        config->node_primary.len,   config->node_primary.p,
        config->node_secondary.len, config->node_secondary.p,
        config->web_root_path.len,  config->web_root_path.p,
        config->db_path.len,        config->db_path.p,
        config->cert.len,           config->cert.p,
        config->key.len,            config->key.p,
        config->log.len,            config->log.p);
    // clang-format on
}
