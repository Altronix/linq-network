#include "linq_daemon.h"
#include "log.h"

static void
print_usage_and_exit(int err)
{
    void* io = err ? stderr : stdout;
    fprintf(
        io,
        "linqd 0.0.1\n"
        "Welcome to linqd\n\n"
        "USAGE:\n\tlinqd [-zsdwh?]\n\n"
        "EXAMPLE:\n\tlinqd -z 33248 -s 8080 -w /etc/www -d ./sqlite.db\n\n"
        "FLAGS:\n"
        "\t-z ZMTP port to listen for incoming devices\n"
        "\t-p Serve HTTP API on port \n"
        "\t-s Server HTTPS API on secure port\n"
        "\t-k TLS key directory\n"
        "\t-c TLS cert file\n"
        "\t-d Database location on local drive\n"
        "\t   Will create a database if one does not already exist\n"
        "\t-w Webpage ROOT\n");
    exit(err);
}

static void
parse_args(linqd_config_s* config, int argc, char* argv[])
{
    int opt, arglen;
    optind = 0;
    while ((opt = getopt(argc, argv, "zpsdck?h")) != -1) {
        switch (opt) {
            case 'z': config->zmtp = atoi(argv[optind]); break;
            case 'p': config->http = atoi(argv[optind]); break;
            case 's': config->https = atoi(argv[optind]); break;
            case 'd': config->db_path = argv[optind]; break;
            case 'c': config->cert = argv[optind]; break;
            case 'k': config->key = argv[optind]; break;
            case '?':
            case 'h':
            default: print_usage_and_exit(0); break;
        }
    }
}

int
main(int argc, char* argv[])
{
    int err = 0;
    linqd_config_s config = { .zmtp = 33248,
                              .http = 8000,
                              .https = 0,
                              .cert = NULL,
                              .key = NULL,
                              .db_path = "./test.db" };
    parse_args(&config, argc, argv);
    if (config.cert && config.key && config.https) {
        // TODO install tls
    }

    linqd_s linqd;
    linqd_init(&linqd, &config);

    while (sys_running()) { err = linqd_poll(&linqd, 5); };

    linqd_free(&linqd);
    return 0;
}
