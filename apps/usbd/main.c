#include "linq_usbd.h"
#include "log.h"
#include "sys.h"

#ifndef USBD_LOG_DEFAULT
#define USBD_LOG_DEFAULT "/var/log/usbd.log"
#endif

typedef struct usbd_config_s
{
    bool daemon;
    const char* log;
} usbd_config_s;

static volatile int running = 1;

void
ctrlc(int dummy)
{
    running = 0;
}

void
sighup(int dummy)
{}

const char* usage = "Usage: atx-update [-pchd]\n"
                    "  -l  log file (default: " USBD_LOG_DEFAULT ")\n"
                    "  -d  Detatch in daemon mode\n"
                    "  -h  Print this help menu\n";

static void
print_usage_and_exit(int err)
{
    void* io = err ? stderr : stdout;
    fprintf(io, "%s", usage);
    exit(err);
}

static void
args_parse(usbd_config_s* config, int argc, char* argv[])
{
    int opt, arglen;
    optind_set(0);
    while ((opt = getopt(argc, argv, "ld?h")) != -1) {
        switch (opt) {
            case 'd': config->daemon = true;
            case 'l': config->log = argv[optind]; break;
            case '?':
            case 'h':
            default: print_usage_and_exit(0); break;
        }
    }
}

static void
usbd_event(
    linq_usbd_s* usb,
    E_USB_EVENTS e,
    const char* b,
    uint32_t l,
    void* ctx)
{
    log_debug("(USBD) Received [%.*s", l, b);
}

struct linq_usbd_callbacks_s callbacks = { .event = &usbd_event };

int
main(int argc, char* argv[])
{
    usbd_config_s config = { .log = USBD_LOG_DEFAULT, .daemon = false };
    args_parse(&config, argc, argv);
    sys_file* f = NULL;
    sys_pid pid = 0;
    linq_usbd_s usb;

    if (config.daemon) sys_daemonize(config.log, &f, &pid);
    linq_usbd_init(&usb, &callbacks, NULL);
    return 0;
}
