#ifndef LINQUSBH_H
#define LINQUSBH_H

#include "linq_usbh.h"
#include <napi.h>

class LinqUsbh : public Napi::ObjectWrap<LinqUsbh>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    LinqUsbh(const Napi::CallbackInfo& info);
    ~LinqUsbh();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value Version(const Napi::CallbackInfo& info);
    Napi::Value Scan(const Napi::CallbackInfo& info);
    void shutdown();
    bool shutdown_;
    linq_usbh_s usbh;
};

#endif /* LINQUSBH_H */
