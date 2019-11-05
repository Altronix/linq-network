#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/linq.h"

static bool received_response = false;

static void
on_response(void* ctx, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)ctx);
    ((void)e);
    ((void)d);
    printf("[S] Received response %s", json);
    received_response = true;
}

static void
on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
{
    ((void)ctx);
    ((void)e);
    ((void)what);
    ((void)serial);
    printf("%s", "[C] Received Error");
}

static void
on_alert(void* ctx, linq_alert_s* alert, linq_email_s* mail, device_s** d)
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
    linq_s* linq = ctx;
    linq_device_send_get(linq, serial, "/ATX/about", on_response, NULL);
}

linq_callbacks callbacks = { .err = on_error,
                             .alert = on_alert,
                             .hb = on_heartbeat };

void
on_request_complete(void* pass, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)e);
    ((void)json);
    ((void)d);
    if (!e && !memcmp("{\"hello\":\"world\"}", json, 17)) {
        *((bool*)pass) = true;
        printf("%s", "[C] received response");
    } else {
        printf("%s", "[C] received response error!");
    }
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = -1;
    linq_socket s;

    linq_s* server = linq_create(&callbacks, NULL);
    if (!server) { return -1; }

    s = linq_listen(server, "tcp://*:33455");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        linq_destroy(&server);
        return -1;
    } else {
        printf("%s", "[S] Listening on port 33455...\n");
    }

    bool request_sent = false;
    while (sys_running()) {
        err = linq_poll(server, 5);
        // if (err) break;

        if (!request_sent && linq_device_count(server)) {
            // printf("%s", "[C] Request Sent!");
            // linq_device_send_get(
            //     server, "dummy", "/ATX/hello", on_request_complete, &pass);
            // request_sent = true;
        }
    }

    linq_destroy(&server);

    return 0;
}

