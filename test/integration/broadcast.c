#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/linq.h"
#include "fixture.h"

static bool received_new_device = false;
static bool received_alert = false;

static void
on_error(void* count, E_LINQ_ERROR e, const char* what, const char* serial)
{
    (*(int*)count)++;
    ((void)e);
    ((void)what);
    ((void)serial);
    printf("%s", "[S] Received Error");
}

static void
on_alert(void* count, linq_alert_s* alert, linq_email_s* mail, device_s** d)
{
    (*(uint32_t*)count)++;
    received_alert = true;
    ((void)alert);
    ((void)mail);
    ((void)d);
    printf("%s", "[S] Received alert\n");
}

static void
on_heartbeat(void* count, const char* serial, device_s** d)
{
    (*(uint32_t*)count)++;
    received_new_device = true;
    ((void)serial);
    ((void)d);
    printf("%s", "[S] Received new device\n");
}

linq_callbacks callbacks = { .err = on_error,
                             .alert = on_alert,
                             .hb = on_heartbeat };

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = -1, count = 0;

    err = 0; // TODO

    fixture_context* fixture = fixture_create(32820);
    if (!fixture) return -1;

    linq_s* server = linq_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    linq_s* client = linq_create(&callbacks, &count);
    if (!client) {
        fixture_destroy(&fixture);
        linq_destroy(&server);
        return -1;
    }

    err = linq_listen(server, "tcp://127.0.0.1:32820");
    if (err) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    // TODO - linq_listen(server, "ipc://server");
    if (err) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    // TODO - linq_connect(client, "ipc://server");
    if (err) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_destroy(&server);
        linq_destroy(&client);
        return -1;
    }

    /*
    // TODO
    while (!(received_new_device && received_alert)) {
        fixture_poll(fixture);
        err = linq_poll(server);
        if (err) break;
        err = linq_poll(client);
        if (err) break;
    }
    */

    fixture_destroy(&fixture);
    linq_destroy(&server);
    linq_destroy(&client);

    return err;
}
