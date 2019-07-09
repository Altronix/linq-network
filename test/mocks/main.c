#include "altronix/linq.h"
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
    czmq_spy_push_incoming_mesg(&a);
    czmq_spy_push_incoming_mesg(&b);
    czmq_spy_push_incoming_mesg(&c);
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
    czmq_spy_push_incoming_mesg(&a);
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
    czmq_spy_push_outgoing_mesg(&a);
    czmq_spy_push_outgoing_mesg(&b);
    czmq_spy_push_outgoing_mesg(&c);
    assert_null(a);
    assert_null(b);
    assert_null(c);
    a = czmq_spy_pop_outgoing_mesg(NULL);
    b = czmq_spy_pop_outgoing_mesg(NULL);
    c = czmq_spy_pop_outgoing_mesg(NULL);
    assert_non_null(a);
    assert_non_null(b);
    assert_non_null(c);
    zmsg_destroy(&a);
    zmsg_destroy(&b);
    zmsg_destroy(&c);
    a = helpers_make_legacy_alert();
    assert_non_null(a);
    czmq_spy_push_outgoing_mesg(&a);
    a = czmq_spy_pop_outgoing_mesg(NULL);
    assert_non_null(a);
    zmsg_destroy(&a);
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
