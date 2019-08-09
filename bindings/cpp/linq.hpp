#ifndef LINQ_HPP_
#define LINQ_HPP_

#include <functional>
#include <memory>
#include <string>

#include "altronix/linq.h"

namespace altronix {

class Device;

static void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
static void on_heartbeat_fn(void*, const char*, device_s**);
static void on_alert_fn(void*, linq_alert_s*, linq_email_s*, device_s**);

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

class Linq
{
  public:
    Linq() { 
        linq_ = linq_create(&callbacks_, this); 
	callbacks_.err = on_error_fn;
	callbacks_.hb = on_heartbeat_fn;
	callbacks_.alert = on_alert_fn;
    }

    ~Linq() { linq_destroy(&linq_); }

    // open up port for device conections
    linq_socket listen(const char* str) { return linq_listen(linq_, str); }

    // connect to a remote linq node
    linq_socket connect(const char* str) { return linq_connect(linq_, str); }

    // shutdown a listener
    void shutdown(linq_socket s) { linq_shutdown(linq_, s); }

    // close connection to a remote node
    void disconnect(linq_socket s) { linq_disconnect(linq_, s); }

    // process io
    E_LINQ_ERROR poll() { return linq_poll(linq_); }

    // get a device context with serial number
    std::shared_ptr<Device> device_get(const char* str)
    {
        device_s** d = linq_device(linq_, str);
        return d ? std::make_shared<Device>(*d) : nullptr;
    }

    // get number of devices connected to linq
    uint32_t device_count() { return linq_device_count(linq_); }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*, Device&)> fn)
    {
        heartbeat_ = std::bind(fn, _1, _2);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(
        std::function<void(linq_alert_s*, linq_email_s*, Device&)> fn)
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

    friend void on_error_fn(void*, E_LINQ_ERROR, const char*, const char*);
    friend void on_heartbeat_fn(void*, const char*, device_s**);
    friend void on_alert_fn(void*, linq_alert_s*, linq_email_s*, device_s**);

  private:
    std::function<void(const char*, Device&)> heartbeat_;
    std::function<void(linq_alert_s*, linq_email_s*, Device&)> alert_;
    std::function<void(E_LINQ_ERROR, const char*, const char*)> error_;
    linq_s* linq_;
    linq_callbacks callbacks_ = {};
};

static void
on_error_fn(void* context, E_LINQ_ERROR e, const char* what, const char* serial)
{
    altronix::Linq* l = (altronix::Linq*)context;
    l->error_(e, what, serial);
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
    linq_alert_s* alert,
    linq_email_s* email,
    device_s** d)
{
    altronix::Linq* l = (altronix::Linq*)context;
    Device device{ *d };
    l->alert_(alert, email, device);
}

} // namespace altronix
#endif /* LINQ_HPP_ */
