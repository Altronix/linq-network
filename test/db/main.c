// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "altronix/linq_netw.h"
#include "db.h"
#include "linq_netw_internal.h"
#include "mock_sqlite.h"

#include <cmocka.h>
#include <setjmp.h>

static void
test_db_create(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_db_create) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
