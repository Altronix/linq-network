// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "fixture.h"
#include "netw.h"

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
on_alert(void* ctx, const char* serial, netw_alert_s* alert, netw_email_s* mail)
{
    ((void)ctx);
    ((void)alert);
    ((void)mail);
    printf("%s", "[C] Received alert\n");
}

static void
on_heartbeat(void* ctx, const char* serial)
{
    ((void)ctx);
    ((void)serial);
    printf("%s", "[C] Received new device\n");
}

netw_callbacks callbacks = { .on_err = on_error,
                             .on_alert = on_alert,
                             .on_heartbeat = on_heartbeat };

void
on_request_complete(
    void* pass,
    const char* serial,
    E_LINQ_ERROR e,
    const char* json)
{
    ((void)e);
    ((void)json);
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
    netw_socket s;
    bool pass = false;

    err = 0; // TODO

    fixture_context* fixture = fixture_create("dummy", 32820);
    if (!fixture) return -1;

    netw_s* server = netw_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    netw_s* client = netw_create(&callbacks, NULL);
    if (!client) {
        fixture_destroy(&fixture);
        netw_destroy(&server);
        return -1;
    }

    s = netw_listen(server, "tcp://127.0.0.1:32820");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        netw_destroy(&server);
        netw_destroy(&client);
        return -1;
    }

    s = netw_listen(server, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        netw_destroy(&server);
        netw_destroy(&client);
        return -1;
    }

    s = netw_connect(client, "ipc:///tmp/request");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[C] Connect Failure!\n");
        fixture_destroy(&fixture);
        netw_destroy(&server);
        netw_destroy(&client);
        return -1;
    }

    bool request_sent = false;
    while (!(pass)) {
        fixture_poll(fixture);
        err = netw_poll(server, 5);
        if (err) break;
        err = netw_poll(client, 5);
        if (err) break;

        if (!request_sent && netw_device_count(client)) {
            printf("%s", "[C] Request Sent!");
            netw_send(
                client,
                "dummy",
                "GET",
                "/ATX/hello",
                10,
                NULL,
                0,
                on_request_complete,
                &pass);
            request_sent = true;
        }
    }

    err = pass ? 0 : -1;

    fixture_destroy(&fixture);
    netw_destroy(&server);
    netw_destroy(&client);

    return err;
}
