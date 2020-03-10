#include "linq_daemon_js.h"
#include "napi.h"
#include <iostream>

#define _NTHROW(__env, __err)                                                  \
    do {                                                                       \
        Napi::TypeError::New(__env, __err).ThrowAsJavaScriptException();       \
    } while (0)

Napi::FunctionReference LinqDaemon::constructor;

Napi::Object
LinqDaemon::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "LinqDaemon",
        {
            InstanceMethod("start", &LinqDaemon::Start),
            InstanceMethod("poll", &LinqDaemon::Poll),
            InstanceMethod("isRunning", &LinqDaemon::IsRunning),
        });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("LinqDaemon", func);
    return exports;
}

LinqDaemon::LinqDaemon(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<LinqDaemon>(info)
{}

LinqDaemon::~LinqDaemon()
{
    linqd_free(&linqd_);
}

Napi::Value
LinqDaemon::Start(const Napi::CallbackInfo& info)
{
    // Initialize some sane default values for initializing the daemon
    config_ = { .zmtp = 33455,
                .http = 8000,
                .https = 8001,
                .db_path = "./test.db",
                .cert = "",
                .key = "" };

    // Get nodejs parameters
    Napi::Env env = info.Env();
    if (!(info.Length() >= 1)) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsObject())) _NTHROW(env, "Expect arg[0] as Object!");
    Napi::Object obj = info[0].ToObject();

    // Merge nodejs caller config with config
    if (obj.Has("zmtp")) config_.zmtp = obj.Get("zmtp").ToNumber();
    if (obj.Has("http")) config_.http = obj.Get("http").ToNumber();
    if (obj.Has("https")) config_.http = obj.Get("https").ToNumber();
    if (obj.Has("db")) {
        config_.db_path = std::string{ obj.Get("db").ToString() }.c_str();
    }
    if (obj.Has("cert")) {
        cert_ = obj.Get("cert").ToString();
        config_.cert = cert_.c_str();
    }
    if (obj.Has("key")) {
        key_ = obj.Get("key").ToString();
        config_.key = key_.c_str();
    }

    // Start deamon
    linqd_init(&linqd_, &config_);
    return info.This();
}

Napi::Value
LinqDaemon::Poll(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as String!");
    uint32_t ms = info[0].ToNumber();
    linqd_poll(&this->linqd_, ms);
    return info.Env().Null();
}

Napi::Value
LinqDaemon::IsRunning(const Napi::CallbackInfo& info)
{
    return Napi::Boolean::New(info.Env(), !this->linqd_.shutdown);
}
