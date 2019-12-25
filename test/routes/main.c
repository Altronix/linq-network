#include "altronix/linq_netw.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "alerts.h"
#include "devices.h"
#include "proxy.h"

void
test_init()
{
    mongoose_spy_init();
    sqlite_spy_init();
    sqlite_spy_step_return_push(SQLITE_DONE); // PRAGMA
    sqlite_spy_step_return_push(SQLITE_ROW);  // device database OK
    sqlite_spy_step_return_push(SQLITE_ROW);  // device database OK
}

void
test_reset()
{
    mongoose_spy_deinit();
    sqlite_spy_deinit();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_route_alerts),
        cmocka_unit_test(test_route_alerts_response_too_large),
        cmocka_unit_test(test_route_alerts_response_get_only),
        cmocka_unit_test(test_route_alerts_response_empty),
        cmocka_unit_test(test_route_devices),
        cmocka_unit_test(test_route_devices_response_too_large),
        cmocka_unit_test(test_route_devices_response_get_only),
        cmocka_unit_test(test_route_devices_response_empty),
        cmocka_unit_test(test_route_proxy)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
