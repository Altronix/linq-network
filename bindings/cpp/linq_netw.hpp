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

#include "altronix/linq_netw.h"

namespace altronix {

class Device;

static void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
static void on_heartbeat_fn(void*, const char*, device_s**);
static void
on_alert_fn(void*, linq_netw_alert_s*, linq_netw_email_s*, device_s**);
static void on_ctrlc_fn(void*);

using namespace std::placeholders;

class Device
{
  public:
    Device(device_s* d)
        : device_{ d } {};

    const char* serial() { return device_serial(device_); }

  private:
    device_s* device_;
};

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
        linq_netw_ = linq_netw_create(&callbacks_, this);
        callbacks_.err = on_error_fn;
        callbacks_.hb = on_heartbeat_fn;
        callbacks_.alert = on_alert_fn;
        callbacks_.ctrlc = on_ctrlc_fn;
    }

    ~Linq() { linq_netw_destroy(&linq_netw_); }

    // open up port for device conections
    linq_netw_socket listen(const char* str)
    {
        return linq_netw_listen(linq_netw_, str);
    }

    // connect to a remote linq node
    linq_netw_socket connect(const char* str)
    {
        return linq_netw_connect(linq_netw_, str);
    }

    // shutdown a listener
    void close_router(linq_netw_socket s)
    {
        linq_netw_close_router(linq_netw_, s);
    }

    // close connection to a remote node
    void close_dealer(linq_netw_socket s)
    {
        linq_netw_close_dealer(linq_netw_, s);
    }

    //
    void close_http(linq_netw_socket socket)
    {
        linq_netw_close_http(linq_netw_, socket);
    }

    // process io
    E_LINQ_ERROR poll(uint32_t ms) { return linq_netw_poll(linq_netw_, ms); }

    static void
    on_response(void* context, E_LINQ_ERROR e, const char* json, device_s** d_p)
    {
        ((void)d_p);
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
            linq_netw_device_send_post(
                this->linq_netw_,
                serial.c_str(),
                path.c_str(),
                data.c_str(),
                on_response,
                response);
        } else if (meth == "DELETE") {
            linq_netw_device_send_delete(
                this->linq_netw_,
                serial.c_str(),
                path.c_str(),
                on_response,
                response);
        } else {
            linq_netw_device_send_get(
                this->linq_netw_,
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

    struct foreach_device_populate_vector_context
    {
        const Linq* linq;
        std::vector<std::shared_ptr<Device>>* devices;
    };

    static void
    foreach_device_populate_vector(void* ctx, const char* sid, const char* type)
    {
        ((void)type);
        foreach_device_populate_vector_context* context =
            static_cast<foreach_device_populate_vector_context*>(ctx);
        context->devices->push_back(context->linq->device(sid));
    }

    std::vector<std::shared_ptr<Device>> devices()
    {
        std::vector<std::shared_ptr<Device>> vec{};
        foreach_device_populate_vector_context context{ this, &vec };
        linq_netw_devices_foreach(
            this->linq_netw_, foreach_device_populate_vector, &context);
        return vec;
    }

    // get a device context with serial number
    std::shared_ptr<Device> device(const char* str) const
    {
        device_s** d = linq_netw_device(linq_netw_, str);
        return d ? std::make_shared<Device>(*d) : nullptr;
    }

    // get number of devices connected to linq
    uint32_t device_count() { return linq_netw_device_count(linq_netw_); }

    uint32_t node_count() { return linq_netw_node_count(linq_netw_); }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*, Device&)> fn)
    {
        heartbeat_ = std::bind(fn, _1, _2);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(
        std::function<void(linq_netw_alert_s*, linq_netw_email_s*, Device&)> fn)
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
    friend void on_heartbeat_fn(void*, const char*, device_s**);
    friend void
    on_alert_fn(void*, linq_netw_alert_s*, linq_netw_email_s*, device_s**);
    friend void on_ctrlc_fn(void*);

  private:
    std::function<void(const char*, Device&)> heartbeat_;
    std::function<void(linq_netw_alert_s*, linq_netw_email_s*, Device&)> alert_;
    std::function<void(E_LINQ_ERROR, const char*, const char*)> error_;
    std::function<void()> ctrlc_;
    linq_netw_s* linq_netw_;
    linq_netw_callbacks callbacks_ = {};
};

static void
on_error_fn(void* context, E_LINQ_ERROR e, const char* what, const char* serial)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->error_) l->error_(e, what, serial);
}

static void
on_heartbeat_fn(void* context, const char* serial, device_s** d)
{
    altronix::Linq* l = (altronix::Linq*)context;
    Device device{ *d };
    if (l->heartbeat_) l->heartbeat_(serial, device);
}

static void
on_alert_fn(
    void* context,
    linq_netw_alert_s* alert,
    linq_netw_email_s* email,
    device_s** d)
{
    altronix::Linq* l = (altronix::Linq*)context;
    Device device{ *d };
    if (l->alert_) l->alert_(alert, email, device);
}

static void
on_ctrlc_fn(void* context)
{
    altronix::Linq* l = (altronix::Linq*)context;
    if (l->ctrlc_) l->ctrlc_();
}

} // namespace altronix
#endif /* LINQ_HPP_ */
