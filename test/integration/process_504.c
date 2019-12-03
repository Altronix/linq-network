// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "altronix/linq_netw.h"
#include "fixture.h"

void
on_request_complete(void* pass, E_LINQ_ERROR e, const char* json, device_s** d)
{
    ((void)json);
    ((void)d);
    *((bool*)pass) = e == LINQ_ERROR_OK ? true : false;
}

static int PORT = 32820;

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    linq_netw_socket s;
    bool pass = false;

    // Create test fixture
    fixture_context* fixture = fixture_create("dummy", PORT);
    if (!fixture) return -1;

    // Create server
    linq_netw_s* server = linq_netw_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    // Listen to port
    char endpoint[32];
    snprintf(endpoint, sizeof(endpoint), "tcp://127.0.0.1:%d", PORT);
    s = linq_netw_listen(server, endpoint);
    if (s == LINQ_ERROR_SOCKET) {
        fixture_destroy(&fixture);
        linq_netw_destroy(&server);
        return -1;
    }

    // Wait for device to connect to us. Then send a request to the test
    // endpoint that sends us 504 as a response a few times.
    bool request_sent = false;
    while (!pass) {
        fixture_poll(fixture);
        if (linq_netw_poll(server, 0)) break;
        if (!request_sent && linq_netw_device_count(server)) {
            linq_netw_device_send_get(
                server, "dummy", "/ATX/test_504", on_request_complete, &pass);
            request_sent = true;
            printf("%s", "[C] Request Sent!");
        }
    }

    fixture_destroy(&fixture);
    linq_netw_destroy(&server);
    return pass ? 0 : -1;
}
