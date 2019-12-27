#include "linqnetwork.h"
#include <mutex>
#include <thread>

Napi::FunctionReference LinqNetwork::constructor;

Napi::Object
LinqNetwork::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "LinqNetwork",
        { InstanceMethod("version", &LinqNetwork::Version),
          InstanceMethod("listen", &LinqNetwork::Version),
          InstanceMethod("closeRouter", &LinqNetwork::Version),
          InstanceMethod("closeDealer", &LinqNetwork::Version),
          InstanceMethod("closeHttp", &LinqNetwork::Version),
          InstanceMethod("device", &LinqNetwork::Version),
          InstanceMethod("deviceCount", &LinqNetwork::Version),
          InstanceMethod("nodeCount", &LinqNetwork::Version),
          InstanceMethod("send", &LinqNetwork::Version) });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("LinqNetwork", func);
    return exports;
}

LinqNetwork::LinqNetwork(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<LinqNetwork>(info)
{}

LinqNetwork::~LinqNetwork() {}

Napi::Value
LinqNetwork::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "0.0.1");
}

Napi::Value
LinqNetwork::Listen(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::CloseRouter(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::CloseDealer(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::CloseHttp(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::Device(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::DeviceCount(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::NodeCount(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::Send(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

std::thread
LinqNetwork::spawn()
{
    return std::thread(&LinqNetwork::process, this);
}

void
LinqNetwork::process()
{}
