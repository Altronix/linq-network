#include "config.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

static int count = 0;

int
__wrap_stat(const char* path, struct stat* statbuff)
{
    if (count < 0) {
        return -1;
    } else if (count == 0) {
        statbuff->st_mode = S_IFREG;
        return 0;
    } else {
        count--;
        return -1;
    }
}

static void
test_sys_config_dir(void** context_p)
{
    const char* result;

    count = 0;
    result = sys_config_dir("foo");
    assert_string_equal(result, "~/.config/altronix/foo.json");

    count = 1;
    result = sys_config_dir("foo");
    assert_string_equal(result, "~/.config/atx/foo.json");

    count = 2;
    result = sys_config_dir("foo");
    assert_string_equal(result, "/etc/atx.config.foo.json");

    count = -1;
    result = sys_config_dir("foo");
    assert_null(result);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_sys_config_dir) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
