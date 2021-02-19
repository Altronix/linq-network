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

#include "netw.h"

namespace altronix {

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
        this->callbacks_.on_err = &Linq::zmtp_error_callback;
        this->callbacks_.on_new = &Linq::zmtp_new_callback;
        this->callbacks_.on_heartbeat = &Linq::zmtp_hb_callback;
        this->callbacks_.on_alert = &Linq::zmtp_alert_callback;
        this->callbacks_.on_ctrlc = &Linq::zmtp_ctrlc_callback;
        netw_ = netw_create(&callbacks_, this);
    }

    ~Linq()
    {
        if (netw_) netw_destroy(&netw_);
    }

    std::string version() { return netw_version(); }

    static void zmtp_error_callback(
        void* context,
        E_LINQ_ERROR e,
        const char* what,
        const char* serial)
    {
        altronix::Linq* l = (altronix::Linq*)context;
        if (l->on_error_) l->on_error_(e, what, serial);
    }

    static void zmtp_hb_callback(void* context, const char* serial)
    {
        altronix::Linq* l = (altronix::Linq*)context;
        if (l->on_heartbeat_) l->on_heartbeat_(serial);
    }

    static void zmtp_new_callback(void* context, const char* serial)
    {
        altronix::Linq* l = (altronix::Linq*)context;
        if (l->on_new_) l->on_new_(serial);
    }

    static void zmtp_alert_callback(
        void* context,
        const char* serial,
        netw_alert_s* alert,
        netw_email_s* email)
    {
        altronix::Linq* l = (altronix::Linq*)context;
        if (l->on_alert_) l->on_alert_(serial, alert, email);
    }

    static void zmtp_ctrlc_callback(void* context)
    {
        altronix::Linq* l = (altronix::Linq*)context;
        if (l->on_ctrlc_) l->on_ctrlc_();
    }

    void early_destruct()
    {
        if (netw_) netw_destroy(&netw_);
    }

    void root(const char* str) { return netw_root(netw_, str); }

    // open up port for device conections
    netw_socket listen(const char* str) { return netw_listen(netw_, str); }

    // connect to a remote linq node
    netw_socket connect(const char* str) { return netw_connect(netw_, str); }

    void close(netw_socket s) { netw_close(netw_, s); }

    // process io
    E_LINQ_ERROR poll(uint32_t ms) { return netw_poll(netw_, ms); }

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
            netw_send(
                this->netw_,
                serial.c_str(),
                "POST",
                path.c_str(),
                path.length(),
                data.c_str(),
                data.length(),
                on_response,
                response);
        } else if (meth == "DELETE") {
            netw_send(
                this->netw_,
                serial.c_str(),
                "DELETE",
                path.c_str(),
                path.length(),
                NULL,
                0,
                on_response,
                response);
        } else {
            netw_send(
                this->netw_,
                serial.c_str(),
                "GET",
                path.c_str(),
                path.length(),
                NULL,
                0,
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
        return netw_device_exists(netw_, sid);
    }

    // get number of devices connected to linq
    uint32_t device_count() { return netw_device_count(netw_); }

    std::string devices()
    {
        const char* devices = netw_devices_summary_alloc(this->netw_);
        if (devices) {
            std::string ret{ devices };
            netw_devices_summary_free(&devices);
            return ret;
        } else {
            return "{}";
        }
    }

    int device_remove(std::string& serial)
    {
        return netw_device_remove(netw_, serial.c_str());
    }

    uint32_t node_count() { return netw_node_count(netw_); }

    // call function fn on every heartbeat
    Linq& on_new(std::function<void(const char*)> fn)
    {
        on_new_ = std::bind(fn, _1);
        return *this;
    }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*)> fn)
    {
        on_heartbeat_ = std::bind(fn, _1);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(
        std::function<void(const char*, netw_alert_s*, netw_email_s*)> fn)
    {
        on_alert_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    // call function fn on every error
    Linq& on_error(
        std::function<void(E_LINQ_ERROR, const char*, const char*)> fn)
    {
        on_error_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    Linq& on_ctrlc(std::function<void()> fn)
    {
        on_ctrlc_ = std::bind(fn);
        return *this;
    }

    int scan() { return netw_scan(this->netw_); }

  private:
    std::function<void(const char*)> on_new_;
    std::function<void(const char*)> on_heartbeat_;
    std::function<void(const char* serial, netw_alert_s*, netw_email_s*)>
        on_alert_;
    std::function<void(E_LINQ_ERROR, const char*, const char*)> on_error_;
    std::function<void()> on_ctrlc_;
    netw_s* netw_;
    netw_callbacks callbacks_;
};

} // namespace altronix
#endif /* LINQ_HPP_ */
