// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linq_network.h"
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
on_alert(
    void* count,
    linq_network_alert_s* alert,
    linq_network_email_s* mail,
    device_s** d)
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

linq_network_callbacks callbacks = { .err = on_error,
                                     .alert = on_alert,
                                     .hb = on_heartbeat };

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);

    uint32_t count = 0;
    int err;
    linq_network_socket s = 0;
    fixture_context* fixture = fixture_create("serial", 32820);
    if (!fixture) return -1;

    linq_network_s* linq = linq_network_create(&callbacks, &count);
    s = linq_network_listen(linq, "tcp://127.0.0.1:32820");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        linq_network_destroy(&linq);
        return -1;
    }

    while (!(received_new_device && received_alert)) {
        fixture_poll(fixture);
        err = linq_network_poll(linq, 5);
        if (err) break;
    }

    fixture_destroy(&fixture);
    linq_network_destroy(&linq);
    return received_new_device && received_alert ? 0 : -1;
}
