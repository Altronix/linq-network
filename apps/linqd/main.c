#include "log.h"
#include "netw.h"
#include <signal.h>

#ifndef LINQD_LOG_DEFAULT
#define LINQD_LOG_DEFAULT "/var/log/atx-linqd.log"
#endif

static volatile int running = 1;

void
ctrlc(int dummy)
{
    running = 0;
}

void
sighup(int dummy)
{}

typedef struct config_s
{
    uint32_t zmtp;
    uint32_t http;
    uint32_t https;
    const char* web_root_path;
    const char* db_path;
    const char* cert;
    const char* key;
    const char* log;
    bool daemon;
} config_s;

static void
print_usage_and_exit(int err)
{
    void* io = err ? stderr : stdout;
    fprintf(
        io,
        "\n"
        "Welcome to linqd (%s)\n\n"
        "USAGE:\n\tlinqd [-zpskcdDlwvh?]\n\n"
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

static void
parse_args(config_s* config, int argc, char* argv[])
{
    int opt, arglen;
    optind_set(0);
    while ((opt = getopt(argc, argv, "zpskcdDlwvh?")) != -1) {
        switch (opt) {
            case 'z': config->zmtp = atoi(argv[optind]); break;
            case 'p': config->http = atoi(argv[optind]); break;
            case 's': config->https = atoi(argv[optind]); break;
            case 'd': config->daemon = true; break;
            case 'D': config->db_path = argv[optind]; break;
            case 'l': config->log = argv[optind]; break;
            case 'c': config->cert = argv[optind]; break;
            case 'k': config->key = argv[optind]; break;
            case 'w': config->web_root_path = argv[optind]; break;
            case 'v': print_version_and_exit(); break;
            case 'h':
            case '?':
            default: print_usage_and_exit(0); break;
        }
    }
}

int
main(int argc, char* argv[])
{
    signal(SIGINT, ctrlc);
    // signal(SIGHUP, sighup);
    int err = 0;
    char endpoint[128];
    netw_s* netw;
    config_s config = { .zmtp = 33248,
                        .http = 8000,
                        .https = 0,
                        .cert = NULL,
                        .log = LINQD_LOG_DEFAULT,
                        .daemon = false,
                        .key = NULL,
                        .db_path = "./test.db" };
    sys_file* f = NULL;
    sys_pid pid = 0;
    parse_args(&config, argc, argv);

    if (config.daemon) sys_daemonize(config.log, &f, &pid);

    netw = netw_create(NULL, NULL);
    assert(netw);

    if (config.zmtp) {
        snprintf(endpoint, sizeof(endpoint), "tcp://*:%d", config.zmtp);
        netw_listen(netw, endpoint);
    }

    if (config.http) {
        snprintf(endpoint, sizeof(endpoint), "http://*:%d", config.http);
        netw_listen(netw, endpoint);
    }

    if (config.web_root_path) { netw_root(netw, config.web_root_path); }

    if (config.https) {
        if (config.cert && config.key && config.https) {
            // TODO install tls
        }
        snprintf(endpoint, sizeof(endpoint), "%d", config.http);
        // netw_listen_https(netw, endpoint); // TODO
    }

    while (netw_running(netw)) { err = netw_poll(netw, 10); };

    netw_destroy(&netw);
    if (f) sys_close(&f);
    return 0;
}
