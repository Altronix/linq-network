#include "altronix/atx_net.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "routes/routes.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
test_route_login_ok(void** context_p)
{
    ((void)context_p);
    helpers_test_config_s config = { .callbacks = NULL,
                                     .context = NULL,
                                     .zmtp = 32820,
                                     .http = 8000,
                                     .user = USER,
                                     .pass = PASS };
    const char* body_expect = "{\"token\":\"TODO\"}";

    helpers_test_context_s* test = test_init(&config);

    test_reset(&test);
}

void
test_route_login_bad_pass(void** context_p)
{
    ((void)context_p);
}
