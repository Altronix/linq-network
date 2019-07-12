#include "device.h"
#include "linq.hpp"

extern "C"
{
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"

#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>
}

static void
test_linq_create(void** context_p)
{
    ((void)context_p);
    altronix::Linq l;
    ((void)l);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_linq_create) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
