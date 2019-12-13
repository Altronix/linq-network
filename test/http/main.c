// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_netw.h"
#include "http.h"
#include "linq_netw_internal.h"
#include "mock_mongoose.h"
#include "mongoose.h"

#include <cmocka.h>
#include <setjmp.h>

static void
test_http_create(void** c_p)
{
    ((void)c_p);
    http_s http;
    http_init(&http);
    http_deinit(&http);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_http_create) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
