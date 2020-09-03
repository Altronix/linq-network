#include "config.h"
#include "log.h"
#include "netw.h"
#include <signal.h>

static volatile int running = 1;

void
ctrlc(int dummy)
{
    running = 0;
}

void
sighup(int dummy)
{}

static void
print_usage_and_exit(int err)
{
    void* io = err ? stderr : stdout;
    fprintf(
        io,
        "\n"
        "Welcome to linqd (%s)\n\n"
        "USAGE:\n\tlinqd [-zpskcdDlwAPvh?]\n\n"
        "EXAMPLE:\n\tlinqd -z 33248 -s 8080 -w /etc/www -d ./sqlite.db\n\n"
        "FLAGS:\n"
        "\t-z ZMTP port to listen for incoming devices\n"
        "\t-p Serve HTTP API on port \n"
        "\t-s Server HTTPS API on secure port\n"
        "\t-d Detatch in daemon mode\n"
        "\t-D Database location on local drive\n"
        "\t-l Log file\n"
        "\t-c TLS cert file\n"
        "\t-k TLS key directory\n"
        "\t   Will create a database if one does not already exist\n"
        "\t-w Webpage ROOT\n"
        "\t-A S(A)ve config\n"
        "\t-P (P)rint config\n"
        "\t-v Print version and exit\n"
        "\t-h Print help menu and exit\n",
        netw_version());
    exit(err);
}

static void
print_version_and_exit()
{
    fprintf(stdout, "%s\n", netw_version());
    exit(0);
}

static json_value
parse_arg(const char* arg)
{
    json_value v = { .p = arg, .len = strlen(arg) };
    return v;
}

static void
parse_args(config_s* config, int argc, char* argv[])
{
    int opt, arglen;
    optind_set(0);
    while ((opt = getopt(argc, argv, "zpskcdDlwAPvh?")) != -1) {
        switch (opt) {
            case 'z': config->zmtp = atoi(argv[optind]); break;
            case 'p': config->http = atoi(argv[optind]); break;
            case 's': config->https = atoi(argv[optind]); break;
            case 'd': config->daemon = true; break;
            case 'D': config->db = parse_arg(argv[optind]); break;
            case 'l': config->log = parse_arg(argv[optind]); break;
            case 'c': config->cert = parse_arg(argv[optind]); break;
            case 'k': config->key = parse_arg(argv[optind]); break;
            case 'w': config->web_root = parse_arg(argv[optind]); break;
            case 'A': config->save = true; break;
            case 'P': config->print = true; break;
            case 'v': print_version_and_exit(); break;
            case 'h':
            case '?':
            default: print_usage_and_exit(0); break;
        }
    }
}

static void
print_config(config_s* c)
{
    log_info("(APP) Print config requested...");
    log_info("(APP) [        daemon] [%s]", c->daemon ? "true" : "false");
    log_info("(APP) [         print] [%s]", c->print ? "true" : "false");
    log_info("(APP) [          save] [%s]", c->save ? "true" : "false");
    log_info("(APP) [          zmtp] [%d]", c->zmtp);
    log_info("(APP) [          http] [%d]", c->http);
    log_info("(APP) [         https] [%d]", c->https);
    log_info("(APP) [      web_root] [%.*s]", c->web_root.len, c->web_root.p);
    log_info("(APP) [            db] [%.*s]", c->db.len, c->db.p);
    log_info("(APP) [          cert] [%.*s]", c->cert.len, c->cert.p);
    log_info("(APP) [           key] [%.*s]", c->key.len, c->key.p);
    log_info("(APP) [           log] [%.*s]", c->log.len, c->log.p);
    log_info(
        "(APP) [  node_primary] [%.*s]",
        c->node_primary.len,
        c->node_primary.p);
    log_info(
        "(APP) [node_secondary] [%.*s]",
        c->node_secondary.len,
        c->node_secondary.p);
}

int
main(int argc, char* argv[])
{
    signal(SIGINT, ctrlc);
    // signal(SIGHUP, sighup);
    int err = 0;
    char buffer[512];
    netw_s* netw;
    config_s config;
    config_init(&config);
    sys_file* f = NULL;
    sys_pid pid = 0;
    parse_args(&config, argc, argv);

    if (config.print) { print_config(&config); }

    if (config.daemon) {
        snprintf(buffer, sizeof(buffer), "%.*s", config.log.len, config.log.p);
        sys_daemonize(buffer, &f, &pid);
    }

    netw = netw_create(NULL, NULL);
    assert(netw);

    if (config.zmtp) {
        snprintf(buffer, sizeof(buffer), "tcp://*:%d", config.zmtp);
        netw_listen(netw, buffer);
    }

    if (config.http) {
        snprintf(buffer, sizeof(buffer), "http://*:%d", config.http);
        netw_listen(netw, buffer);
    }

    if (config.web_root.p) {
        snprintf(
            buffer,
            sizeof(buffer),
            "%.*s",
            config.web_root.len,
            config.web_root.p);
        netw_root(netw, buffer);
    }

    if (config.https) {
        if (config.cert.p && config.key.p && config.https) {
            // TODO install tls
        }
        snprintf(buffer, sizeof(buffer), "%d", config.http);
        // netw_listen_https(netw, buffer); // TODO
    }

    while (netw_running(netw)) { err = netw_poll(netw, 10); };

    netw_destroy(&netw);
    if (f) sys_close(&f);
    return 0;
}
