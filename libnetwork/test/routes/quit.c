#include "quit.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "netw.h"
#include "routes/routes.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"

void
__wrap_netw_shutdown(netw_s* netw)
{
    *((bool*)netw) = true;
}

void
test_route_quit(void** context_p)
{
    http_request_s r;
    http_route_context route, *route_p = &route;
    memset(&r, 0, sizeof(r));
    mongoose_parser_context* parser;
    bool pass = false;
    char resp[256];
    uint32_t l;
    l = snprintf(resp, 256, "{\"error\":\"%s\"}", http_error_message(0));

    mongoose_spy_init();
    route.context = &pass;
    r.route_p = &route_p;
    route_quit(&r, HTTP_METHOD_GET, 0, NULL);
    parser = mongoose_spy_response_pop();
    assert_true(pass);
    assert_non_null(parser);
    assert_int_equal(parser->content_length, l);
    assert_memory_equal(resp, parser->body, l);
    free(parser);
    mongoose_spy_deinit();
}

