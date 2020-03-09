#include "linq_daemon.h"

static void
print_usage_and_exit(int err)
{
    void* io = err ? stderr : stdout;
    fprintf(io, "linqd 0.0.1\n");
    fprintf(io, "Welcome to linqd\n\n");
    fprintf(io, "USAGE:\n\tlinqd [-zsd]\n\n");
    fprintf(io, "EXAMPLE:\n\tlinqd -z 33248 -s 8080 -d ./sqlite.db\n\n");
    fprintf(io, "FLAGS:\n");
    fprintf(io, "\t-z ZMTP port to listen for incoming devices\n");
    fprintf(io, "\t-s HTTP port to listen for UI\n");
    fprintf(io, "\t-d DATABASE location on local drive\n");
    exit(err);
}

static void
parse_args(linqd_config_s* config, int argc, char* argv[])
{
    int opt, arglen;
    optind = 0;
    while ((opt = getopt(argc, argv, "zsd")) != -1) {
        switch (opt) {
            case 'z': config->zmtp = atoi(argv[optind]); break;
            case 's': config->http = atoi(argv[optind]); break;
            case 'd': config->db_path = argv[optind]; break;
            case '?':
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
                              .db_path = "./test.db" };
    parse_args(&config, argc, argv);

    linqd_s linqd;
    linqd_init(&linqd, &config);

    while (sys_running()) { err = linqd_poll(&linqd, 5); };

    linqd_free(&linqd);
    return 0;
}
