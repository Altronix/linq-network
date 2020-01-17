#include "atxnet.h"
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
        {
            InstanceMethod("version", &LinqNetwork::Version),
            InstanceMethod("registerCallback", &LinqNetwork::RegisterCallback),
            InstanceMethod("isRunning", &LinqNetwork::IsRunning),
            InstanceMethod("poll", &LinqNetwork::Poll),
            InstanceMethod("listen", &LinqNetwork::Listen),
            InstanceMethod("closeRouter", &LinqNetwork::CloseRouter),
            InstanceMethod("closeDealer", &LinqNetwork::CloseDealer),
            InstanceMethod("closeHttp", &LinqNetwork::CloseHttp),
            InstanceMethod("deviceCount", &LinqNetwork::DeviceCount),
            InstanceMethod("nodeCount", &LinqNetwork::NodeCount),
            InstanceMethod("send", &LinqNetwork::Send),
            InstanceMethod("sendGet", &LinqNetwork::Get),
            InstanceMethod("sendPost", &LinqNetwork::Post),
            InstanceMethod("sendDelete", &LinqNetwork::Del),
        });
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
                auto env = this->r_callback_.Env();
                auto hb = Napi::String::New(env, "heartbeat");
                auto sid = Napi::String::New(env, serial);
                this->r_callback_.Call({ hb, sid });
            }
        })
        .on_alert([this](
                      atx_net_alert_s* alert,
                      atx_net_email_s* email,
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
    this->r_callback_ = Napi::Persistent(info[0].As<Napi::Function>());
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
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as Number!");
    uint32_t arg0 = info[0].ToNumber();
    this->linq_.close_router(arg0);
    return info.Env().Null();
}

Napi::Value
LinqNetwork::CloseDealer(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as Number!");
    uint32_t arg0 = info[0].ToNumber();
    this->linq_.close_dealer(arg0);
    return info.Env().Null();
}

Napi::Value
LinqNetwork::CloseHttp(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as Number!");
    uint32_t arg0 = info[0].ToNumber();
    this->linq_.close_http(arg0);
    return info.Env().Null();
}

Napi::Value
LinqNetwork::DeviceCount(const Napi::CallbackInfo& info)
{
    return Napi::Number::New(info.Env(), this->linq_.device_count());
}

Napi::Value
LinqNetwork::NodeCount(const Napi::CallbackInfo& info)
{
    return Napi::Number::New(info.Env(), this->linq_.node_count());
}

Napi::Value
LinqNetwork::Get(const Napi::CallbackInfo& info)
{
    // Napi::Env env = info.Env();
    Napi::Env env = Env();
    if (!(info.Length() >= 2)) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");
    if (!(info[1].IsString())) _NTHROW(env, "Expect arg[1] as String!");
    std::string sid = info[0].ToString();
    std::string path = info[1].ToString();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
    this->linq_.get(sid, path, [=](altronix::Response& response) {
        auto env = Env();
        auto err = Napi::Number::New(env, response.error);
        auto json = Napi::String::New(env, response.response);
        deferred.Resolve(json);
    });
    return deferred.Promise();
}

Napi::Value
LinqNetwork::Post(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::Del(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
LinqNetwork::Send(const Napi::CallbackInfo& info)
{
    // Napi::Env env = info.Env();
    Napi::Env env = Env();
    if (!(info.Length() >= 4)) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");
    if (!(info[1].IsString())) _NTHROW(env, "Expect arg[1] as String!");
    if (!(info[2].IsString())) _NTHROW(env, "Expect arg[2] as String!");
    if (!(info[3].IsString())) _NTHROW(env, "Expect arg[3] as String!");
    std::string sid = info[0].ToString();
    std::string meth = info[1].ToString();
    std::string path = info[2].ToString();
    std::string data = info[3].ToString();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
    this->linq_.send(sid, meth, path, data, [=](altronix::Response& response) {
        auto env = Env();
        auto err = Napi::Number::New(env, response.error);
        auto json = Napi::String::New(env, response.response);
        deferred.Resolve(json);
    });
    return deferred.Promise();
}

void
LinqNetwork::shutdown()
{
    this->shutdown_ = true;
}
