#ifndef LINQNETWORK_H
#define LINQNETWORK_H

#include "altronix/linq_netw.hpp"
#include <napi.h>

#include <mutex>
#include <thread>

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
    Napi::Value Listen(const Napi::CallbackInfo& info);
    Napi::Value CloseRouter(const Napi::CallbackInfo& info);
    Napi::Value CloseDealer(const Napi::CallbackInfo& info);
    Napi::Value CloseHttp(const Napi::CallbackInfo& info);
    Napi::Value Device(const Napi::CallbackInfo& info);
    Napi::Value DeviceCount(const Napi::CallbackInfo& info);
    Napi::Value NodeCount(const Napi::CallbackInfo& info);
    Napi::Value Send(const Napi::CallbackInfo& info);

    // Thread
    std::thread spawn();
    void process();

    // Private variables
    std::thread t_;
    std::mutex m_;
    altronix::Linq linq_;
};

#endif
