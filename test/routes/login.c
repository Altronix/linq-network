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
}

void
test_route_login_bad_pass(void** context_p)
{
    ((void)context_p);
}
