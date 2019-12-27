#include "linqnetwork.h"
#include <chrono>
#include <mutex>
#include <thread>

#define _NTHROW(__env, __err)                                                  \
    do {                                                                       \
        Napi::TypeError::New(__env, __err).ThrowAsJavaScriptException();       \
    } while (0)

Napi::FunctionReference LinqNetwork::constructor;

Napi::Object
LinqNetwork::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "LinqNetwork",
        { InstanceMethod("version", &LinqNetwork::Version),
          InstanceMethod("listen", &LinqNetwork::Listen),
          InstanceMethod("closeRouter", &LinqNetwork::CloseRouter),
          InstanceMethod("closeDealer", &LinqNetwork::CloseDealer),
          InstanceMethod("closeHttp", &LinqNetwork::CloseHttp),
          InstanceMethod("device", &LinqNetwork::Device),
          InstanceMethod("deviceCount", &LinqNetwork::DeviceCount),
          InstanceMethod("nodeCount", &LinqNetwork::NodeCount),
          InstanceMethod("send", &LinqNetwork::Send) });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("LinqNetwork", func);
    return exports;
}

LinqNetwork::LinqNetwork(const Napi::CallbackInfo& info)
    : shutdown_{ false }
    , Napi::ObjectWrap<LinqNetwork>(info)
{
    this->t_ = this->spawn();
}

LinqNetwork::~LinqNetwork()
{
    this->t_.join();
}

Napi::Value
LinqNetwork::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "0.0.1");
}

Napi::Value
LinqNetwork::Listen(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    // Validate inputs
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");

    // Call c routine with arguments
    std::string arg0 = info[0].ToString();
    std::lock_guard<std::mutex> guard(this->m_);
    auto s = this->linq_.listen(arg0.c_str());
    return Napi::Number::New(info.Env(), s);
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
{
    while (!this->shutdown_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::lock_guard<std::mutex> guard(this->m_);
        this->linq_.poll(0);
    }
}
