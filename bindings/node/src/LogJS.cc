#include "LogJS.h"
#include "netw.h"
#include <napi.h>

Napi::FunctionReference Logger::constructor;

Napi::Object
Logger::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "Logger",
        {
            InstanceMethod("open", &Logger::Open),
            InstanceMethod("close", &Logger::Close),
        });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Logger", func);
    return exports;
}

Logger::Logger(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Logger>(info)
{}

Logger::~Logger() {}

Napi::Value
Logger::Open(const Napi::CallbackInfo& info)
{
    if (info.Length() >= 1 && info[0].IsFunction()) {
        this->callback = Napi::Persistent(info[0].As<Napi::Function>());
        netw_log_fn_set(this->logs, this);
    }
    return info.This();
}

Napi::Value
Logger::Close(const Napi::CallbackInfo& info)
{
    return info.This();
}
