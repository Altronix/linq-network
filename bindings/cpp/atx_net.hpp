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

#include "altronix/atx_net.h"

namespace altronix {

class Device;

static void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
static void on_heartbeat_fn(void*, const char*, device_s**);
static void
on_alert_fn(void*, atx_net_alert_s*, atx_net_email_s*, device_s**);
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
        atx_net_ = atx_net_create(&callbacks_, this);
        callbacks_.err = on_error_fn;
        callbacks_.hb = on_heartbeat_fn;
        callbacks_.alert = on_alert_fn;
        callbacks_.ctrlc = on_ctrlc_fn;
    }

    ~Linq() { atx_net_destroy(&atx_net_); }

    // open up port for device conections
    atx_net_socket listen(const char* str)
    {
        return atx_net_listen(atx_net_, str);
    }

    // connect to a remote linq node
    atx_net_socket connect(const char* str)
    {
        return atx_net_connect(atx_net_, str);
    }

    // shutdown a listener
    void close_router(atx_net_socket s)
    {
        atx_net_close_router(atx_net_, s);
    }

    // close connection to a remote node
    void close_dealer(atx_net_socket s)
    {
        atx_net_close_dealer(atx_net_, s);
    }

    //
    void close_http(atx_net_socket socket)
    {
        atx_net_close_http(atx_net_, socket);
    }

    // process io
    E_LINQ_ERROR poll(uint32_t ms) { return atx_net_poll(atx_net_, ms); }

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
            atx_net_send_post(
                this->atx_net_,
                serial.c_str(),
                path.c_str(),
                data.c_str(),
                on_response,
                response);
        } else if (meth == "DELETE") {
            atx_net_send_delete(
                this->atx_net_,
                serial.c_str(),
                path.c_str(),
                on_response,
                response);
        } else {
            atx_net_send_get(
                this->atx_net_,
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
        atx_net_devices_foreach(
            this->atx_net_, foreach_device_populate_vector, &context);
        return vec;
    }

    // get a device context with serial number
    std::shared_ptr<Device> device(const char* str) const
    {
        device_s** d = atx_net_device(atx_net_, str);
        return d ? std::make_shared<Device>(*d) : nullptr;
    }

    // get number of devices connected to linq
    uint32_t device_count() { return atx_net_device_count(atx_net_); }

    uint32_t node_count() { return atx_net_node_count(atx_net_); }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*, Device&)> fn)
    {
        heartbeat_ = std::bind(fn, _1, _2);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(
        std::function<void(atx_net_alert_s*, atx_net_email_s*, Device&)> fn)
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
    on_alert_fn(void*, atx_net_alert_s*, atx_net_email_s*, device_s**);
    friend void on_ctrlc_fn(void*);

  private:
    std::function<void(const char*, Device&)> heartbeat_;
    std::function<void(atx_net_alert_s*, atx_net_email_s*, Device&)> alert_;
    std::function<void(E_LINQ_ERROR, const char*, const char*)> error_;
    std::function<void()> ctrlc_;
    atx_net_s* atx_net_;
    atx_net_callbacks callbacks_ = {};
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
    atx_net_alert_s* alert,
    atx_net_email_s* email,
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
