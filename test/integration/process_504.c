// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixture.h"
#include "linq_network.h"

void
on_request_complete(
    void* pass,
    const char* serial,
    E_LINQ_ERROR e,
    const char* json)
{
    ((void)json);
    *((bool*)pass) = e == LINQ_ERROR_OK ? true : false;
}

static int PORT = 32820;

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    linq_network_socket s;
    bool pass = false;

    // Create test fixture
    fixture_context* fixture = fixture_create("dummy", PORT);
    if (!fixture) return -1;

    // Create server
    linq_network_s* server = linq_network_create(NULL, NULL);
    if (!server) {
        fixture_destroy(&fixture);
        return -1;
    }

    // Listen to port
    char endpoint[32];
    snprintf(endpoint, sizeof(endpoint), "tcp://127.0.0.1:%d", PORT);
    s = linq_network_listen(server, endpoint);
    if (s == LINQ_ERROR_SOCKET) {
        fixture_destroy(&fixture);
        linq_network_destroy(&server);
        return -1;
    }

    // Wait for device to connect to us. Then send a request to the test
    // endpoint that sends us 504 as a response a few times.
    bool request_sent = false;
    while (!pass) {
        fixture_poll(fixture);
        if (linq_network_poll(server, 0)) break;
        if (!request_sent && linq_network_device_count(server)) {
            linq_network_send(
                server,
                "dummy",
                "GET",
                "/ATX/test_504",
                13,
                NULL,
                0,
                on_request_complete,
                &pass);
            request_sent = true;
            printf("%s", "[C] Request Sent!");
        }
    }

    fixture_destroy(&fixture);
    linq_network_destroy(&server);
    return pass ? 0 : -1;
}
