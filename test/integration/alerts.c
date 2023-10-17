// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixture.h"
#include "netw.h"

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
    const char* serial,
    netw_alert_s* alert,
    netw_email_s* mail)
{
    (*(uint32_t*)count)++;
    received_alert = true;
    ((void)alert);
    ((void)mail);
    printf("%s", "[S] Received alert\n");
}

static void
on_heartbeat(void* count, const char* serial)
{
    (*(uint32_t*)count)++;
    received_new_device = true;
    ((void)serial);
    printf("%s", "[S] Received new device\n");
}

netw_callbacks callbacks = { .on_err = on_error,
                             .on_alert = on_alert,
                             .on_heartbeat = on_heartbeat };

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);

    uint32_t count = 0;
    int err;
    netw_socket s = 0;
    fixture_context* fixture = fixture_create("serial", 32820);
    if (!fixture) return -1;

    netw_s* linq = netw_create(&callbacks, &count);
    s = netw_listen(linq, "tcp://127.0.0.1:32820");
    if (s == LINQ_ERROR_SOCKET) {
        printf("%s", "[S] Listen Failure!\n");
        fixture_destroy(&fixture);
        netw_destroy(&linq);
        return -1;
    }

    while (!(received_new_device && received_alert)) {
        fixture_poll(fixture);
        err = netw_poll(linq, 5);
        if (err) break;
    }

    fixture_destroy(&fixture);
    netw_destroy(&linq);
    return received_new_device && received_alert ? 0 : -1;
}
