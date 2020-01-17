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
on_alert(
    void* ctx,
    atx_net_alert_s* alert,
    atx_net_email_s* mail,
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
    ((void)ctx);
    ((void)serial);
    ((void)d);
    printf("%s", "[C] Received new device\n");
}

atx_net_callbacks callbacks = { .err = on_error,
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
    atx_net_socket s;
    bool pass = false;

    err = 0; // TODO

    fixture_context* fixture = fixture_create("dummy", 32820);
    if (!fixture) return -1;

    atx_net_s* server = atx_net_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    atx_net_s* client = atx_net_create(&callbacks, NULL);
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

    s = atx_net_listen(server, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        atx_net_destroy(&client);
        return -1;
    }

    s = atx_net_connect(client, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[C] Connect Failure!\n");
        fixture_destroy(&fixture);
        atx_net_destroy(&server);
        atx_net_destroy(&client);
        return -1;
    }

    bool request_sent = false;
    while (!(pass)) {
        fixture_poll(fixture);
        err = atx_net_poll(server, 5);
        if (err) break;
        err = atx_net_poll(client, 5);
        if (err) break;

        if (!request_sent && atx_net_device_count(client)) {
            printf("%s", "[C] Request Sent!");
            atx_net_device_send_get(
                client, "dummy", "/ATX/hello", on_request_complete, &pass);
            request_sent = true;
        }
    }

    err = pass ? 0 : -1;

    fixture_destroy(&fixture);
    atx_net_destroy(&server);
    atx_net_destroy(&client);

    return err;
}
