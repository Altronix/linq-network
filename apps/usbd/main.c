#include "linq_usbd.h"
#include "log.h"
#include "sys.h"
#include <signal.h>

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
            case 'd': config->daemon = true; break;
            case 'l': config->log = argv[optind]; break;
            case '?':
            case 'h':
            default: print_usage_and_exit(0); break;
        }
    }
}

static void
usbd_event(linq_usbd_s* usb, void* ctx, E_USB_EVENTS e, ...)
{
    if (USB_EVENTS_TYPE_HTTP == e) {
        const char *meth, *path, *data;
        va_list list;
        va_start(list, e);
        meth = va_arg(list, const char*);
        path = va_arg(list, const char*);
        data = va_arg(list, const char*);
        log_info("(USB) RECV [%s] [%s] [%s]", meth, path, data ? data : "");
        linq_usbd_write_http_response(usb, 200, "{\"error\":\"Ok\"}");
        va_end(list);
    } else if (USB_EVENTS_ERROR == e) {
        int ret;
        va_list list;
        va_start(list, e);
        ret = va_arg(list, int);
        va_end(list);
        log_info("(USB) error [%d]", ret);
    }
}

int
main(int argc, char* argv[])
{
    usbd_config_s config = { .log = USBD_LOG_DEFAULT, .daemon = false };
    signal(SIGINT, ctrlc);
    signal(SIGHUP, sighup);
    args_parse(&config, argc, argv);
    sys_file* f = NULL;
    sys_pid pid = 0;
    linq_usbd_s usb;

    if (config.daemon) { sys_daemonize(config.log, &f, &pid); }
    linq_usbd_init(&usb);

    while (running) {
        sys_msleep(50);
        linq_usbd_poll(&usb, usbd_event, &usb);
    }

    linq_usbd_free(&usb);
    return 0;
}
