#include "json.h"
#include "sys.h"
#include "wire.h"

#include <setjmp.h>

#include "mock_file.h"

#include <cmocka.h>

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {

    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}

