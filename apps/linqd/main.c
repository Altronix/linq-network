#include "config.h"
#include "libcommon/config.h"
#include "log.h"
#include "netw.h"
#include "route_config.h"
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
        "USAGE:\n\tlinqd [-zpskcdDlnwCPvh?]\n\n"
        "EXAMPLE:\n\tlinqd -z 33248 -s 8080 -w /etc/www -d ./sqlite.db\n\n"
        "FLAGS:\n"
        "\t-z ZMTP port to listen for incoming devices\n"
        "\t-p Serve HTTP API on port \n"
        "\t-s Server HTTPS API on secure port\n"
        "\t-d Detatch in daemon mode\n"
        "\t-D Database location on local drive\n"
        "\t-l Log file\n"
        "\t-n Connect to node\n"
        "\t-c TLS cert file\n"
        "\t-k TLS key directory\n"
        "\t   Will create a database if one does not already exist\n"
        "\t-w Webpage ROOT\n"
        "\t-C Config\n"
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
    while ((opt = getopt(argc, argv, "zpskcdDlnwCPvh?")) != -1) {
        switch (opt) {
            case 'z': config->zmtp = atoi(argv[optind]); break;
            case 'p': config->http = atoi(argv[optind]); break;
            case 's': config->https = atoi(argv[optind]); break;
            case 'd': config->daemon = true; break;
            case 'D': config->db = parse_arg(argv[optind]); break;
            case 'l': config->log = parse_arg(argv[optind]); break;
            case 'n': config->node_primary = parse_arg(argv[optind]); break;
            case 'c': config->cert = parse_arg(argv[optind]); break;
            case 'k': config->key = parse_arg(argv[optind]); break;
            case 'w': config->web_root = parse_arg(argv[optind]); break;
            case 'C': config->save = parse_arg(argv[optind]); break;
            case 'P': config->print = true; break;
            case 'v': print_version_and_exit(); break;
            case 'h':
            case '?':
            default: print_usage_and_exit(0); break;
        }
    }
}

static int
parse_config_file(config_s* config)
{
    int rc;
    char buff[1024];
    uint32_t l = sizeof(buff);
    const char* dir;
    sys_file* f = NULL;

    // Find config file
    dir = sys_config_dir("linqd");
    if (!dir) return ENOENT;

    // Open config file
    f = sys_open(dir, FILE_MODE_READ, FILE_BLOCKING);
    if (!f) return EIO;

    // Read config file
    rc = sys_read_buffer(f, buff, &l);
    if ((rc <= 0)) {
        sys_close(&f);
        return EIO;
    }

    // Parse config file
    rc = config_parse(buff, l, config);
    if (rc) {
        sys_close(&f);
        return EIO;
    }
    return 0;
}

static void
print_config(config_s* c)
{
    log_info("(APP) Print config requested...");
    log_info("(APP) [        daemon] [%s]", c->daemon ? "true" : "false");
    log_info("(APP) [         print] [%s]", c->print ? "true" : "false");
    log_info("(APP) [          zmtp] [%d]", c->zmtp);
    log_info("(APP) [          http] [%d]", c->http);
    log_info("(APP) [         https] [%d]", c->https);
    log_info("(APP) [          save] [%.*s]", c->save.len, c->save.p);
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
    char b[512];
    netw_s* netw;
    sys_file* f = NULL;
    sys_pid pid = 0;
    config_s config;

    // Load configuration from defaults
    config_init(&config);

    // Load configuration from config file (overrides default)
    err = parse_config_file(&config);
    if (err) {}

    // Load configuration from command line (overrides default and config file)
    parse_args(&config, argc, argv);

    if (config.print) { print_config(&config); }

    if (config.save.p) {
        snprintf(b, sizeof(b), "%.*s", config.save.len, config.save.p);
        sys_file* f = sys_open(b, FILE_MODE_READ_WRITE_CREATE, FILE_BLOCKING);
        if (f) {
            log_info("(APP) Saving config to [%s]", b);
            config_fprint(f, &config);
            sys_close(&f);
        } else {
            log_error("(APP) Failed to open file for saving!");
        }
    }

    snprintf(b, sizeof(b), "%.*s", config.log.len, config.log.p);
    if (config.daemon) {
        sys_daemonize(b, &f, &pid);
    } else if (config.log.len) {
        f = sys_open(b, FILE_MODE_READ_APPEND_CREATE, FILE_BLOCKING);
        if (f) {
            log_set_fd(f);
            log_set_color(false);
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
        }
    }

    netw = netw_create(NULL, NULL);
    assert(netw);

    if (config.zmtp) {
        snprintf(b, sizeof(b), "tcp://*:%d", config.zmtp);
        netw_listen(netw, b);
    }

    if (config.http) {
        snprintf(b, sizeof(b), "http://*:%d", config.http);
        netw_listen(netw, b);
        netw_use(netw, "/api/v1/config", route_config, netw);
    }

    if (config.node_primary.p) {
        snprintf(
            b,
            sizeof(b),
            "%.*s",
            config.node_primary.len,
            config.node_primary.p);
        netw_connect(netw, b);
    }

    if (config.web_root.p) {
        snprintf(b, sizeof(b), "%.*s", config.web_root.len, config.web_root.p);
        netw_root(netw, b);
    }

    if (config.https) {
        if (config.cert.p && config.key.p && config.https) {
            // TODO install tls
        }
        snprintf(b, sizeof(b), "%d", config.http);
        // netw_listen_https(netw, b); // TODO
    }

    while (netw_running(netw)) { err = netw_poll(netw, 50); };

    netw_destroy(&netw);
    if (f) sys_close(&f);
    return 0;
}
