#include "linqnetwork.h"
#include <chrono>

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
          InstanceMethod("registerCallback", &LinqNetwork::RegisterCallback),
          InstanceMethod("isRunning", &LinqNetwork::IsRunning),
          InstanceMethod("poll", &LinqNetwork::Poll),
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
    this->linq_
        .on_heartbeat([this](const char* serial, altronix::Device& device) {
            ((void)device);
            if (this->r_callback_) {
                // std::lock_guard<std::mutex> guard(this->m_);
                auto env = this->r_callback_.Env();
                auto hb = Napi::String::New(env, "heartbeat");
                auto sid = Napi::String::New(env, serial);
                this->r_callback_.Call({ hb, sid });
            }
        })
        .on_alert([this](
                      linq_netw_alert_s* alert,
                      linq_netw_email_s* email,
                      altronix::Device& d) {
            ((void)alert);
            ((void)email);
            ((void)d);
        })
        .on_error(
            [this](E_LINQ_ERROR error, const char* serial, const char* err) {
                ((void)error);
                ((void)serial);
                ((void)err);
            })
        .on_ctrlc([this]() { this->shutdown(); });
}

LinqNetwork::~LinqNetwork() {}

Napi::Value
LinqNetwork::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "0.0.1");
}

Napi::Value
LinqNetwork::RegisterCallback(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (!(info.Length()) >= 1) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsFunction())) _NTHROW(env, "Expect arg[0] as Function!");

    // TESTING
    this->r_callback_ = Napi::Persistent(info[0].As<Napi::Function>());
    auto ref_env = this->r_callback_.Env();
    auto hb = Napi::String::New(ref_env, "heartbeat");
    auto sid = Napi::String::New(ref_env, "serial");
    this->r_callback_.Call({ hb, sid });

    return env.Null();
}

Napi::Value
LinqNetwork::IsRunning(const Napi::CallbackInfo& info)
{
    return Napi::Boolean::New(info.Env(), !this->shutdown_);
}

Napi::Value
LinqNetwork::Poll(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as String!");
    uint32_t ms = info[0].ToNumber();
    this->linq_.poll(ms);
    return info.Env().Null();
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

void
LinqNetwork::shutdown()
{
    this->shutdown_ = true;
}
