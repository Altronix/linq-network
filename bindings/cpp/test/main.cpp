// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "device.h"
#include "linq_netw.hpp"

extern "C"
{
#include "helpers.h"
#include "linq_netw_internal.h"
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
test_linq_netw_create(void** context_p)
{
    ((void)context_p);
    altronix::Linq l;
    ((void)l);
}

static void
test_linq_netw_device(void** context_p)
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
test_linq_netw_alert(void** context_p)
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
    l.on_alert([&alert_pass](
                   linq_netw_alert_s* alert,
                   linq_netw_email_s* email,
                   altronix::Device& d) {
        assert_string_equal(d.serial(), "serial");
        assert_memory_equal(alert->who.p, "TestUser", 8);
        assert_memory_equal(alert->what.p, "TestAlert", 9);
        assert_memory_equal(alert->where.p, "Altronix Site ID", 16);
        assert_memory_equal(alert->when.p, "1", 1);
        assert_memory_equal(alert->mesg.p, "Test Alert Message", 18);
        assert_memory_equal(email->to0.p, "mail0@gmail.com", 15);
        assert_memory_equal(email->to1.p, "mail1@gmail.com", 15);
        assert_memory_equal(email->to2.p, "mail2@gmail.com", 15);
        assert_memory_equal(email->to3.p, "mail3@gmail.com", 15);
        assert_memory_equal(email->to4.p, "mail4@gmail.com", 15);
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
    const struct CMUnitTest tests[] = { cmocka_unit_test(test_linq_netw_device),
                                        cmocka_unit_test(test_linq_netw_create),
                                        cmocka_unit_test(
                                            test_linq_netw_alert) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
