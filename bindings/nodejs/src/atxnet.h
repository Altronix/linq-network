#ifndef LINQNETWORK_H
#define LINQNETWORK_H

#include "atx_net.hpp"
#include <napi.h>

class LinqNetwork : public Napi::ObjectWrap<LinqNetwork>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    LinqNetwork(const Napi::CallbackInfo& info);
    ~LinqNetwork();

  private:
    // Binding functions
    static Napi::FunctionReference constructor;
    Napi::Value Version(const Napi::CallbackInfo& info);
    Napi::Value RegisterCallback(const Napi::CallbackInfo& info);
    Napi::Value IsRunning(const Napi::CallbackInfo& info);
    Napi::Value Poll(const Napi::CallbackInfo& info);
    Napi::Value Listen(const Napi::CallbackInfo& info);
    Napi::Value CloseRouter(const Napi::CallbackInfo& info);
    Napi::Value CloseDealer(const Napi::CallbackInfo& info);
    Napi::Value CloseHttp(const Napi::CallbackInfo& info);
    Napi::Value Device(const Napi::CallbackInfo& info);
    Napi::Value DeviceCount(const Napi::CallbackInfo& info);
    Napi::Value NodeCount(const Napi::CallbackInfo& info);
    Napi::Value Send(const Napi::CallbackInfo& info);
    Napi::Value Get(const Napi::CallbackInfo& info);
    Napi::Value Post(const Napi::CallbackInfo& info);
    Napi::Value Del(const Napi::CallbackInfo& info);

    // Thread
    void shutdown();

    // Private variables
    bool shutdown_;
    altronix::Linq linq_;
    Napi::Function emit_;
    Napi::FunctionReference r_callback_;
};

#endif
