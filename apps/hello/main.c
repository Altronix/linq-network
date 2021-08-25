#include "netw.h"
#include <signal.h>

static volatile int running = 1;

static int done = 0;

static int hello = 0;

void
ctrlc(int dummy)
{
    running = 0;
}

void
sighup(int dummy)
{}

static void
on_response(void* context, const char* serial, E_LINQ_ERROR e, const char* json)
{
    printf("ok\n");
    done = 1;
}

static void
send_hello(netw_s* l, const char* sid)
{
    int e = netw_send(l, sid, "GET", "/ATX/about", 10, NULL, 0, on_response, l);
    assert(e == 0);
    hello = 1;
}

static void
on_heartbeat_fn(void* context, const char* serial)
{
    netw_s* linq = context;
    if (!hello) send_hello(linq, serial);
}

netw_callbacks callbacks = { .on_heartbeat = on_heartbeat_fn,
                             .on_alert = NULL,
                             .on_new = NULL,
                             .on_err = NULL,
                             .on_ctrlc = NULL };

int
main(int argc, char* argv[])
{
    signal(SIGINT, ctrlc);
    int err = 0;
    char b[512];
    netw_s* netw;

    netw = netw_create(&callbacks, NULL);
    assert(netw);
    netw_context_set(netw, netw);

    netw_listen(netw, "tcp://*:33455");

    while (running && !done) { err = netw_poll(netw, 50); };

    netw_destroy(&netw);
    return 0;
}
