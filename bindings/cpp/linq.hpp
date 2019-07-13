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

    e_linq_error listen(const char* str) { return linq_listen(linq_, str); }
    void poll() { linq_poll(linq_); }
    device** device_get(const char* str) { return linq_device(linq_, str); }
    uint32_t device_count() { return linq_device_count(linq_); }

    friend void on_error_fn(void*, e_linq_error, const char*, const char*);
    friend void on_heartbeat_fn(void*, const char*, device**);
    friend void on_alert_fn(void*, linq_alert*, linq_email*, device**);

    Linq& on_heartbeat(std::function<void(const char*, device**)> fn)
    {
        heartbeat_ = std::bind(fn, _1, _2);
        return *this;
    }

  private:
    std::function<void(const char*, device**)> heartbeat_;

    linq* linq_;
    linq_callbacks callbacks_ = { .err = on_error_fn,
                                  .hb = on_heartbeat_fn,
                                  .alert = on_alert_fn };
};

void
on_error_fn(void* ctx, e_linq_error e, const char* what, const char* serial)
{
    ((void)ctx);
    ((void)e);
    ((void)what);
    ((void)serial);
}

void
on_heartbeat_fn(void* context, const char* serial, device** d)
{
    Linq* l = (Linq*)context;
    if (l->heartbeat_) l->heartbeat_(serial, d);
}

void
on_alert_fn(void* pass, linq_alert* alert, linq_email* email, device** d)
{
    ((void)pass);
    ((void)alert);
    ((void)email);
    ((void)d);
}

} // namespace altronix

#endif /* LINQ_HPP_ */
