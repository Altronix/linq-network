#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/linq_netw.h"

static bool received_response = false;

static void
on_response(void* ctx, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)ctx);
    ((void)e);
    ((void)d);
    printf("[S] Received response [%d] %s\n", e, json);
    received_response = true;
}

static void
on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    ((void)ctx);
    ((void)e);
    ((void)what);
    ((void)serial);
    printf("%s", "[C] Received Error\n");
}

static void
on_alert(
    void* ctx,
    linq_netw_alert_s* alert,
    linq_netw_email_s* mail,
    device_s** d)
{
    ((void)ctx);
    ((void)alert);
    ((void)mail);
    ((void)d);
    printf("%s", "[C] Received alert\n");
}

static void
on_heartbeat(void* ctx, const char* serial, device_s** d)
{
    ((void)serial);
    ((void)d);
    printf("%s", "[C] Received new device\n");
    linq_netw_s* linq = ctx;
    linq_netw_device_send_get(linq, serial, "/ATX/abosut", on_response, NULL);
}

linq_netw_callbacks callbacks = { .err = on_error,
                                  .alert = on_alert,
                                  .hb = on_heartbeat };

void
on_request_complete(void* pass, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)d);
    if (!e && !memcmp("{\"hello\":\"world\"}", json, 17)) {
        *((bool*)pass) = true;
        printf("%s [%d]", "[C] received response\n", e);
    } else {
        printf("%s [%d]", "[C] received response error!\n", e);
    }
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = -1;
    linq_netw_socket s;

    linq_netw_s* server = linq_netw_create(&callbacks, NULL);
    if (!server) { return -1; }

    s = linq_netw_listen(server, "tcp://*:33455");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        linq_netw_destroy(&server);
        return -1;
    } else {
        printf("%s", "[S] Listening on port 33455...\n");
    }

    bool request_sent = false;
    while (sys_running()) {
        err = linq_netw_poll(server, 5);
        // if (err) break;

        if (!request_sent && linq_netw_device_count(server)) {
            // printf("%s", "[C] Request Sent!");
            // linq_netw_device_send_get(
            //     server, "dummy", "/ATX/hello", on_request_complete, &pass);
            // request_sent = true;
        }
    }

    linq_netw_destroy(&server);

    return 0;
}

