// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LINQ_HPP_
#define LINQ_HPP_

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "linq_network.h"

namespace altronix {

static void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
static void on_heartbeat_fn(void*, const char*);
static void
on_alert_fn(void*, const char*, linq_network_alert_s*, linq_network_email_s*);
static void on_ctrlc_fn(void*);

using namespace std::placeholders;

struct Response
{
    E_LINQ_ERROR error;
    std::string response;
    std::function<void(Response&)> fn;
};

class Linq
{
  public:
    Linq()
    {
        linq_network_ = linq_network_create(&callbacks_, this);
        callbacks_.err = on_error_fn;
        callbacks_.hb = on_heartbeat_fn;
        callbacks_.alert = on_alert_fn;
        callbacks_.ctrlc = on_ctrlc_fn;
    }

    ~Linq() { linq_network_destroy(&linq_network_); }

    // open up port for device conections
    linq_network_socket listen(const char* str)
    {
        return linq_network_listen(linq_network_, str);
    }

    // connect to a remote linq node
    linq_network_socket connect(const char* str)
    {
        return linq_network_connect(linq_network_, str);
    }

    void close(linq_network_socket s) { linq_network_close(linq_network_, s); }

    // process io
    E_LINQ_ERROR poll(uint32_t ms)
    {
        return linq_network_poll(linq_network_, ms);
    }

    static void on_response(
        void* context,
        const char* serial,
        E_LINQ_ERROR e,
        const char* json)
    {
        Response* r = static_cast<Response*>(context);
        r->error = e;
        r->response = std::string(json);
        r->fn(*r);
        delete r;
    }

    void send(
        std::string serial,
        std::string meth,
        std::string path,
        std::string data,
        std::function<void(Response&)> fn)
    {
        Response* response = new Response{ LINQ_ERROR_OK, "", fn };
        if (meth == "POST" || meth == "PUT") {
            linq_network_send_post(
                this->linq_network_,
                serial.c_str(),
                path.c_str(),
                data.c_str(),
                on_response,
                response);
        } else if (meth == "DELETE") {
            linq_network_send_delete(
                this->linq_network_,
                serial.c_str(),
                path.c_str(),
                on_response,
                response);
        } else {
            linq_network_send_get(
                this->linq_network_,
                serial.c_str(),
                path.c_str(),
                on_response,
                response);
        }
    }

    void send(
        std::string serial,
        std::string meth,
        std::string path,
        std::function<void(Response&)> fn)
    {
        this->send(serial, meth, path, "", fn);
    }

    void
    get(std::string serial, std::string path, std::function<void(Response&)> fn)
    {
        this->send(serial, "GET", path, "", fn);
    }

    void post(
        std::string serial,
        std::string path,
        std::string data,
        std::function<void(Response&)> fn)
    {
        this->send(serial, "POST", path, data, fn);
    }

    void
    del(std::string serial, std::string path, std::function<void(Response&)> fn)
    {
        this->send(serial, "DELETE", path, fn);
    }

    bool device_exists(const char* sid)
    {
        return linq_network_device_exists(linq_network_, sid);
    }

    // get number of devices connected to linq
    uint32_t device_count() { return linq_network_device_count(linq_network_); }

    uint32_t node_count() { return linq_network_node_count(linq_network_); }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*)> fn)
    {
        heartbeat_ = std::bind(fn, _1);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(
        std::function<
            void(const char*, linq_network_alert_s*, linq_network_email_s*)> fn)
    {
        alert_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    // call function fn on every error
    Linq& on_error(
        std::function<void(E_LINQ_ERROR, const char*, const char*)> fn)
    {
        error_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    Linq& on_ctrlc(std::function<void()> fn)
    {
        ctrlc_ = std::bind(fn);
        return *this;
    }

    friend void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
    friend void on_heartbeat_fn(void*, const char*);
    friend void on_alert_fn(
        void*,
        const char*,
        linq_network_alert_s*,
        linq_network_email_s*);
    friend void on_ctrlc_fn(void*);

  private:
    std::function<void(const char*)> heartbeat_;
    std::function<
        void(const char* serial, linq_network_alert_s*, linq_network_email_s*)>
        alert_;
    std::function<void(E_LINQ_ERROR, const char*, const char*)> error_;
    std::function<void()> ctrlc_;
    linq_network_s* linq_network_;
    linq_network_callbacks callbacks_ = {};
};

static void
on_error_fn(void* context, E_LINQ_ERROR e, const char* what, const char* serial)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->error_) l->error_(e, what, serial);
}

static void
on_heartbeat_fn(void* context, const char* serial)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->heartbeat_) l->heartbeat_(serial);
}

static void
on_alert_fn(
    void* context,
    const char* serial,
    linq_network_alert_s* alert,
    linq_network_email_s* email)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->alert_) l->alert_(serial, alert, email);
}

static void
on_ctrlc_fn(void* context)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->ctrlc_) l->ctrlc_();
}

} // namespace altronix
#endif /* LINQ_HPP_ */
