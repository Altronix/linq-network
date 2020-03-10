#ifndef LINQDAEMON_H
#define LINQDAEMON_H

#include "linq_daemon.h"
#include <napi.h>

class LinqDaemon : public Napi::ObjectWrap<LinqDaemon>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    LinqDaemon(const Napi::CallbackInfo& info);
    ~LinqDaemon();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value Start(const Napi::CallbackInfo& info);
    Napi::Value Poll(const Napi::CallbackInfo& info);
    Napi::Value IsRunning(const Napi::CallbackInfo& info);
    linqd_s linqd_;
    linqd_config_s config_;
    std::string cert_;
    std::string key_;
};

#endif
