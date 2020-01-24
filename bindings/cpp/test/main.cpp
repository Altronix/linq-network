// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "atx_net.hpp"
#include "device.h"

extern "C"
{
#include "atx_net_internal.h"
#include "helpers.h"
#include "mock_mongoose.h"
#include "mock_sqlite.h"
#include "mock_zmsg.h"
#include "mock_zpoll.h"

#include <czmq.h>

#include <cmocka.h>
#include <setjmp.h>
}

static void
test_init()
{
    helpers_test_init("unsafe_user", "unsafe_password");
}

static void
test_reset()
{
    helpers_test_reset();
}

static void
test_atx_net_create(void** context_p)
{
    ((void)context_p);

    test_init();

    altronix::Linq l;
    ((void)l);

    test_reset();
}

static void
test_atx_net_device(void** context_p)
{
    ((void)context_p);

    test_init();

    altronix::Linq l;

    zmsg_t* hb = helpers_make_heartbeat("rid", "serial", "pid", "site");
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_poll_set_incoming((0x01));

    l.listen("tcp://*:32999");
    l.poll(5);

    std::shared_ptr<altronix::Device> d = l.device("serial");
    bool pass = d ? true : false;
    assert_true(pass);
    assert_string_equal(d->serial(), "serial");

    test_reset();
}

static void
test_atx_net_devices(void** context_p)
{
    ((void)context_p);

    test_init();

    altronix::Linq l;

    zmsg_t* hb0 = helpers_make_heartbeat("rid0", "serial0", "pid0", "site0");
    zmsg_t* hb1 = helpers_make_heartbeat("rid1", "serial1", "pid1", "site1");
    czmq_spy_mesg_push_incoming(&hb0);
    czmq_spy_mesg_push_incoming(&hb1);
    czmq_spy_poll_set_incoming((0x01));

    l.listen("tcp://*:32999");
    l.poll(5);
    l.poll(5);

    auto devices = l.devices();
    assert_int_equal(devices.size(), 2);
    // bool pass = d ? true : false;
    // assert_true(pass);
    // assert_string_equal(d->serial(), "serial");

    test_reset();
}

static void
test_atx_net_alert(void** context_p)
{
    ((void)context_p);

    test_init();

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
                   atx_net_alert_s* alert,
                   atx_net_email_s* email,
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

void
test_atx_net_send(void** context_p)
{
    ((void)context_p);
    zmsg_t* hb = helpers_make_heartbeat("rid0", "serial0", "pid", "sid");
    zmsg_t* r = helpers_make_response("rid0", "serial0", 0, "{\"response\":0}");
    bool pass = false;

    test_init();
    sqlite_spy_step_return_push(SQLITE_ROW);
    sqlite_spy_column_int_return_push(1);
    czmq_spy_mesg_push_incoming(&hb);
    czmq_spy_mesg_push_incoming(&r);
    czmq_spy_poll_set_incoming((0x01));

    altronix::Linq linq{};
    linq.listen("tcp://*:32820");
    linq.poll(0); // receive heartbeat

    // send get request
    linq.get("serial0", "/ATX/about", [&pass](altronix::Response& response) {
        assert_true(response.response == "{\"response\":0}");
        pass = true;
    });

    // Process response
    linq.poll(0);
    assert_true(pass);
    test_reset();
}

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err;
    const struct CMUnitTest tests[] = //
        { cmocka_unit_test(test_atx_net_device),
          cmocka_unit_test(test_atx_net_devices),
          cmocka_unit_test(test_atx_net_create),
          cmocka_unit_test(test_atx_net_alert),
          cmocka_unit_test(test_atx_net_send) };

    err = cmocka_run_group_tests(tests, NULL, NULL);
    return err;
}
