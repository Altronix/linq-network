#include "quit.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "netw.h"
#include "sys.h"

#include <setjmp.h>

#include <cmocka.h>

#include "main.h"
void quit(
    http_route_context* ctx,
    HTTP_METHOD meth,
    uint32_t jlen,
    const char* body);

void
__wrap_netw_shutdown(netw_s* netw)
{
    *((bool*)netw) = true;
}

void
test_route_quit(void** context_p)
{
    http_route_context ctx;
    memset(&ctx, 0, sizeof(ctx));
    mongoose_parser_context* parser;
    bool pass = false;
    char resp[256];
    uint32_t l;
    l = snprintf(resp, 256, "{\"error\":\"%s\"}", http_error_message(200));

    mongoose_spy_init();
    ctx.context = &pass;
    quit(&ctx, HTTP_METHOD_GET, 0, NULL);
    parser = mongoose_spy_response_pop();
    assert_true(pass);
    assert_non_null(parser);
    assert_int_equal(parser->content_length, l);
    assert_memory_equal(resp, parser->body, l);
    free(parser);
    mongoose_spy_deinit();
}

