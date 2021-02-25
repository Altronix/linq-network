#include "netw.h"

#ifndef SWARM_NODES
#define SWARM_NODES 2
#endif

#ifndef SWARM_ZMTP_START
#define SWARM_ZMTP_START 33240
#endif

#ifndef SWARM_HTTP_START
#define SWARM_HTTP_START 16000
#endif

static void
swarm_init(netw_s** networks)
{
    char buff[128];
    netw_s** netw_p;
    for (int i = 0; i < SWARM_NODES; i++) {
        netw_p = networks + i;
        *netw_p = netw_create(NULL, NULL);

        // Listen for incoming ZMTP
        snprintf(buff, sizeof(buff), "tcp://*:%d", SWARM_ZMTP_START + i);
        netw_listen(*netw_p, buff);

        // Listen for incoming HTTP
        snprintf(buff, sizeof(buff), "http://*:%d", SWARM_HTTP_START + i);
        netw_listen(*netw_p, buff);
    }
}

static void
swarm_connect(netw_s** networks)
{
    char buff[128];
    netw_s** netw_p;
    for (int i = 0; i < SWARM_NODES; i++) {
        netw_p = networks + i;
        // Connect to adjacent nodes
        for (int port = 0; port < SWARM_NODES; port++) {
            if (port == i) continue; // don't connect to ourself
            snprintf(
                buff,
                sizeof(buff),
                "tcp://127.0.0.1:%d",
                SWARM_ZMTP_START + port);
            netw_connect(*netw_p, buff);
        }
    }
}

static void
swarm_poll(netw_s** networks)
{
    for (int i = 0; i < SWARM_NODES; i++) netw_poll(*(networks++), 20);
}

static void
swarm_cleanup(netw_s** networks)
{
    for (int i = 0; i < SWARM_NODES; i++) netw_destroy(networks++);
}

int
main(int argc, char* argv[])
{
    netw_s* networks[SWARM_NODES];

    swarm_init(networks);
    swarm_connect(networks);
    while (netw_running(networks[0])) swarm_poll(networks);
    swarm_cleanup(networks);
}
