#include "linq_network.h"
#include "helpers.h"
#include "mock_utils.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "alerts.h"
#include "create_admin.h"
#include "devices.h"
#include "login.h"
#include "proxy.h"

helpers_test_context_s*
test_init(helpers_test_config_s* config)
{
    return helpers_test_context_create(config);
}

void
test_reset(helpers_test_context_s** test_p)
{
    helpers_test_context_destroy(test_p);
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
        cmocka_unit_test(test_route_proxy_get),
        cmocka_unit_test(test_route_proxy_post),
        cmocka_unit_test(test_route_proxy_404),
        cmocka_unit_test(test_route_proxy_400_too_short),
        cmocka_unit_test(test_route_create_admin_ok),
        cmocka_unit_test(test_route_create_admin_fail_exists),
        cmocka_unit_test(test_route_login_ok),
        cmocka_unit_test(test_route_login_bad_pass),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
