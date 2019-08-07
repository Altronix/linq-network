#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/linq.h"
#include "fixture.h"

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
    ((void)ctx);
    ((void)serial);
    ((void)d);
    printf("%s", "[C] Received new device\n");
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
    bool pass = false;

    err = 0; // TODO

    fixture_context* fixture = fixture_create("dummy", 32820);
    if (!fixture) return -1;

    linq_s* server = linq_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    linq_s* client = linq_create(&callbacks, NULL);
    if (!client) {
        fixture_destroy(&fixture);
        linq_destroy(&server);
        return -1;
    }

    s = linq_listen(server, "tcp://127.0.0.1:32820");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    s = linq_listen(server, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    s = linq_connect(client, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[C] Connect Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    bool request_sent = false;
    while (!(pass)) {
        fixture_poll(fixture);
        err = linq_poll(server);
        if (err) break;
        err = linq_poll(client);
        if (err) break;

        if (!request_sent && linq_device_count(client)) {
            printf("%s", "[C] Request Sent!");
            linq_device_send_get(
                client, "dummy", "/ATX/hello", on_request_complete, &pass);
            request_sent = true;
        }
    }

    err = pass ? 0 : -1;

    fixture_destroy(&fixture);
    linq_destroy(&server);
    linq_destroy(&client);

    return err;
}
