#include "log.h"
#include "netw.h"

#define app_info(...) log_info("TTY", __VA_ARGS__)
#define app_warn(...) log_warn("TTY", __VA_ARGS__)
#define app_debug(...) log_debug("TTY", __VA_ARGS__)
#define app_trace(...) log_trace("TTY", __VA_ARGS__)
#define app_error(...) log_error("TTY", __VA_ARGS__)
#define app_fatal(...) log_fatal("TTY", __VA_ARGS__)

static void
response(void* ctx, const char* serial, E_LINQ_ERROR e, const char* mesg)
{
    app_info("response [%d] [%s]", e, mesg);
    *((bool*)ctx) = true;
}

int
main(int argc, char* argv[])
{
    int rc;
    bool done = false;
    const char* req = "/api/v1/network";
    netw_s* netw = netw_create(NULL, NULL);
    assert(netw);

    rc = netw_scan(netw);
    app_info("usb scan [%d]", rc);

    rc = netw_send(
        netw,
        "linqm5serialnumberhere",
        "GET",
        req,
        4,
        NULL,
        0,
        response,
        &done);
    if (rc < 0) {
        app_error("send error [%d]", rc);
        netw_destroy(&netw);
        exit(rc);
    }

    while (!done) { netw_poll(netw, 50); }

    netw_destroy(&netw);
}
