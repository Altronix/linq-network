#ifndef LINQ_HPP_
#define LINQ_HPP_

#include <functional>
#include <memory>
#include <string>

#include "altronix/linq.h"

namespace altronix {

void on_error_fn(void*, e_linq_error, const char*, const char*);
void on_heartbeat_fn(void*, const char*, device**);
void on_alert_fn(void*, linq_alert*, linq_email*, device**);

using namespace std::placeholders;

class Linq
{
  public:
    Linq() { linq_ = linq_create(&callbacks_, this); }

    ~Linq() { linq_destroy(&linq_); }

    // open up port for device conections
    e_linq_error listen(const char* str) { return linq_listen(linq_, str); }

    // process io
    void poll() { linq_poll(linq_); }

    // get a device context with serial number
    device** device_get(const char* str) { return linq_device(linq_, str); }

    // get number of devices connected to linq
    uint32_t device_count() { return linq_device_count(linq_); }

    // call function fn on every heartbeat
    Linq& on_heartbeat(std::function<void(const char*, device**)> fn)
    {
        heartbeat_ = std::bind(fn, _1, _2);
        return *this;
    }

    // call function fn on every alert
    Linq& on_alert(std::function<void(linq_alert*, linq_email*, device**)> fn)
    {
        alert_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    // call function fn on every error
    Linq& on_error(
        std::function<void(e_linq_error, const char*, const char*)> fn)
    {
        error_ = std::bind(fn, _1, _2, _3);
        return *this;
    }

    friend void on_error_fn(void*, e_linq_error, const char*, const char*);
    friend void on_heartbeat_fn(void*, const char*, device**);
    friend void on_alert_fn(void*, linq_alert*, linq_email*, device**);

  private:
    std::function<void(const char*, device**)> heartbeat_;
    std::function<void(linq_alert*, linq_email*, device**)> alert_;
    std::function<void(e_linq_error, const char*, const char*)> error_;

    linq* linq_;
    linq_callbacks callbacks_ = { .err = on_error_fn,
                                  .hb = on_heartbeat_fn,
                                  .alert = on_alert_fn };
};

void
on_error_fn(void* context, e_linq_error e, const char* what, const char* serial)
{
    Linq* l = (Linq*)context;
    l->error_(e, what, serial);
}

void
on_heartbeat_fn(void* context, const char* serial, device** d)
{
    Linq* l = (Linq*)context;
    if (l->heartbeat_) l->heartbeat_(serial, d);
}

void
on_alert_fn(void* context, linq_alert* alert, linq_email* email, device** d)
{
    Linq* l = (Linq*)context;
    l->alert_(alert, email, d);
}

} // namespace altronix

#endif /* LINQ_HPP_ */
