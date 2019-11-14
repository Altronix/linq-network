#include "altronix/linq_io.h"
#include "helpers.h"
#include "mock_zmsg.h"

#include <setjmp.h>

#include <cmocka.h>

static void
test_mock_push_incoming(void** context_p)
{
    ((void)context_p);
    zmsg_t *a, *b, *c;

    a = helpers_make_legacy_alert();
    b = helpers_make_legacy_alert();
    c = helpers_make_legacy_alert();
    czmq_spy_mesg_push_incoming(&a);
    czmq_spy_mesg_push_incoming(&b);
    czmq_spy_mesg_push_incoming(&c);
    assert_null(a);
    assert_null(b);
    assert_null(c);
    a = zmsg_recv(NULL);
    b = zmsg_recv(NULL);
    c = zmsg_recv(NULL);
    assert_non_null(a);
    assert_non_null(b);
    assert_non_null(c);
    zmsg_destroy(&a);
    zmsg_destroy(&b);
    zmsg_destroy(&c);
    a = helpers_make_legacy_alert();
    assert_non_null(a);
    czmq_spy_mesg_push_incoming(&a);
    a = zmsg_recv(NULL);
    assert_non_null(a);
    zmsg_destroy(&a);
}

static void
test_mock_push_outgoing(void** context_p)
{
    ((void)context_p);
    zmsg_t *a, *b, *c;

    a = helpers_make_legacy_alert();
    b = helpers_make_legacy_alert();
    c = helpers_make_legacy_alert();
    czmq_spy_mesg_push_outgoing(&a);
    czmq_spy_mesg_push_outgoing(&b);
    czmq_spy_mesg_push_outgoing(&c);
    assert_null(a);
    assert_null(b);
    assert_null(c);
    a = czmq_spy_mesg_pop_outgoing(NULL);
    b = czmq_spy_mesg_pop_outgoing(NULL);
    c = czmq_spy_mesg_pop_outgoing(NULL);
    assert_non_null(a);
    assert_non_null(b);
    assert_non_null(c);
    zmsg_destroy(&a);
    zmsg_destroy(&b);
    zmsg_destroy(&c);
    a = helpers_make_legacy_alert();
    assert_non_null(a);
    czmq_spy_mesg_push_outgoing(&a);
    a = czmq_spy_mesg_pop_outgoing(NULL);
    assert_non_null(a);
    zmsg_destroy(&a);
}

static void
test_mock_zsock(void** context_p)
{
    ((void)context_p);
    zsock_t* d = zsock_new_dealer("");
    zsock_t* r = zsock_new_router("");
    assert_non_null(d);
    assert_non_null(r);
    zsock_destroy(&d);
    zsock_destroy(&r);
    assert_null(d);
    assert_null(r);
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mock_push_incoming),
        cmocka_unit_test(test_mock_push_outgoing),
        cmocka_unit_test(test_mock_zsock)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
