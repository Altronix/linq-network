#include "scan.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "netw.h"
#include "routes/routes.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

int
__wrap_netw_scan(netw_s* netw)
{
    return 3;
}

void
test_route_scan(void** context_p)
{
    http_request_s r;
    http_route_context route, *route_p = &route;
    memset(&r, 0, sizeof(r));
    mongoose_parser_context* parser;

    mongoose_spy_init();
    r.route_p = &route_p;
    route_scan(&r, HTTP_METHOD_GET, 0, NULL);
    parser = mongoose_spy_response_pop();
    assert_non_null(parser);
    assert_int_equal(parser->content_length, 11);
    assert_memory_equal("{\"count\":3}", parser->body, 11);
    free(parser);
    mongoose_spy_deinit();
}
