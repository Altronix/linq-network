#include "altronix/linq_netw.h"

#include "helpers.h"
#include "mock_mongoose.h"
#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_alloc(void** context_p)
{
    ((void)context_p);
    mongoose_spy_init();

    mongoose_spy_event_request_push("admin:admin", "GET", "/ATX/hello", NULL);
    mongoose_spy_event_request_push(
        "admin:admin", "POST", "/ATX/hello", "{\"hello\":\"world\"}");

    mongoose_spy_deinit();
}

static void
test_mock_push_incoming(void** context_p)
{
    ((void)context_p);
}

static void
test_mock_push_outgoing(void** context_p)
{
    ((void)context_p);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mock_push_incoming),
        cmocka_unit_test(test_mock_push_outgoing)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
