// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/atx_net.h"

static bool received_response = false;

static void
on_response(void* ctx, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)ctx);
    ((void)e);
    ((void)d);
    ((void)json);
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
    atx_net_alert_s* alert,
    atx_net_email_s* mail,
    device_s** d)
{
    ((void)ctx);
    ((void)alert);
    ((void)mail);
    ((void)d);
}

static void
on_heartbeat(void* ctx, const char* serial, device_s** d)
{
    ((void)ctx);
    ((void)serial);
    ((void)d);
    /*
    atx_net_s* linq = ctx;
    atx_net_device_send_get(linq, serial, "/ATX/test/504", on_response, NULL);
    */
}

atx_net_callbacks callbacks = { .err = on_error,
                                  .alert = on_alert,
                                  .hb = on_heartbeat };

void
on_request_complete(void* pass, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)d);
    if (!e && !memcmp("{\"hello\":\"world\"}", json, 17)) {
        *((bool*)pass) = true;
    } else {
    }
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = -1;
    atx_net_socket s, http;

    atx_net_s* server = atx_net_create(&callbacks, NULL);
    if (!server) { return -1; }

    s = atx_net_listen(server, "tcp://*:33455");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        atx_net_destroy(&server);
        return -1;
    }

    http = atx_net_listen(server, "http://*:8000");
    if (http == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] HTTP Listen Failure!\n");
        atx_net_destroy(&server);
        return -1;
    }

    while (sys_running()) { err = atx_net_poll(server, 5); }

    atx_net_destroy(&server);

    return 0;
}

