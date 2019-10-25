#include "device.h"
#include "linq.hpp"

extern "C"
{
#include "helpers.h"
#include "linq_internal.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"

#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>
}

static void
test_reset()
{
    czmq_spy_mesg_reset();
    czmq_spy_poll_reset();
}

static void
test_linq_create(void** context_p)
{
    ((void)context_p);
    altronix::Linq l;
    ((void)l);
}

static void
test_linq_device(void** context_p)
{
    ((void)context_p);
    altronix::Linq l;

    zmsg_t* hb = helpers_make_heartbeat("rid", "serial", "pid", "site");
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    l.listen("tcp://*:32999");
    l.poll(5);

    std::shared_ptr<altronix::Device> d = l.device_get("serial");
    bool pass = d ? true : false;
    assert_true(pass);
    assert_string_equal(d->serial(), "serial");

    test_reset();
}

static void
test_linq_alert(void** context_p)
{
    ((void)context_p);
    bool alert_pass = false;
    altronix::Linq l;

    zmsg_t* hb = helpers_make_heartbeat("rid", "serial", "pid", "site");
    zmsg_t* alert = helpers_make_alert("rid", "serial", "pid");

    // Push some incoming messages
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&alert);
    czmq_spy_poll_set_incoming((0x01));

    l.listen("tcp://*:32820");
    l.on_alert(
        [&alert_pass](
            linq_alert_s* alert, linq_email_s* email, altronix::Device& d) {
            assert_string_equal(d.serial(), "serial");
            assert_string_equal(alert->who, "TestUser");
            assert_string_equal(alert->what, "TestAlert");
            assert_string_equal(alert->where, "Altronix Site ID");
            assert_string_equal(alert->when, "1");
            assert_string_equal(alert->mesg, "Test Alert Message");
            assert_string_equal(email->to0, "mail0@gmail.com");
            assert_string_equal(email->to1, "mail1@gmail.com");
            assert_string_equal(email->to2, "mail2@gmail.com");
            assert_string_equal(email->to3, "mail3@gmail.com");
            assert_string_equal(email->to4, "mail4@gmail.com");
            alert_pass = true;
        });

    // Read heartbeat and alert
    l.poll(5);
    l.poll(5);
    assert_true(alert_pass);

    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_linq_device),
                                        cmocka_unit_test(test_linq_create),
                                        cmocka_unit_test(test_linq_alert) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
