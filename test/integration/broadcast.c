// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/atx_net.h"
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
    printf("%s", "[C] Received Error");
}

static void
on_alert(
    void* count,
    atx_net_alert_s* alert,
    atx_net_email_s* mail,
    device_s** d)
{
    (*(uint32_t*)count)++;
    received_alert = true;
    ((void)alert);
    ((void)mail);
    ((void)d);
    printf("%s", "[C] Received alert\n");
}

static void
on_heartbeat(void* count, const char* serial, device_s** d)
{
    (*(uint32_t*)count)++;
    received_new_device = true;
    ((void)serial);
    ((void)d);
    printf("%s", "[C] Received new device\n");
}

atx_net_callbacks callbacks = { .err = on_error,
                                  .alert = on_alert,
                                  .hb = on_heartbeat };

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    atx_net_socket s;
    int err = -1, count = 0;

    err = 0; // TODO

    fixture_context* fixture = fixture_create("serial", 32820);
    if (!fixture) return -1;

    atx_net_s* server = atx_net_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    atx_net_s* client = atx_net_create(&callbacks, &count);
    if (!client) {
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        return -1;
    }

    s = atx_net_listen(server, "tcp://127.0.0.1:32820");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        atx_net_destroy(&client);
        return -1;
    }

    s = atx_net_listen(server, "ipc:///tmp/broadcast");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        atx_net_destroy(&client);
        return -1;
    }

    s = atx_net_connect(client, "ipc:///tmp/broadcast");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[C] Connect Failure!\n");
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        atx_net_destroy(&client);
        return -1;
    }

    while (!(received_new_device && received_alert)) {
        fixture_poll(fixture);
        err = atx_net_poll(server, 5);
        if (err) break;
        err = atx_net_poll(client, 5);
        if (err) break;
    }

    err = received_new_device && received_alert ? 0 : -1;

    fixture_destroy(&fixture);
    atx_net_destroy(&server);
    atx_net_destroy(&client);

    return err;
}
