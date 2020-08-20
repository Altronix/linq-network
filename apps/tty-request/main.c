#include "log.h"
#include "netw.h"

static void
response(void* ctx, const char* serial, E_LINQ_ERROR e, const char* mesg)
{
    log_info("(APP) response [%d] [%s]", e, mesg);
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
    log_info("(APP) usb scan [%d]", rc);

    rc = netw_send(netw, "N/A", "GET", req, 4, NULL, 0, response, &done);
    if (rc < 0) {
        log_error("(APP) send error [%d]", rc);
        netw_destroy(&netw);
        exit(rc);
    }

    while (!done) { netw_poll(netw, 50); }

    netw_destroy(&netw);
}
