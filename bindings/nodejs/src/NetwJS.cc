#include "NetwJS.h"
#include "napi.h"
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
            InstanceMethod("earlyDestruct", &LinqNetwork::EarlyDestruct),
            InstanceMethod("version", &LinqNetwork::Version),
            InstanceMethod("registerCallback", &LinqNetwork::RegisterCallback),
            InstanceMethod("isRunning", &LinqNetwork::IsRunning),
            InstanceMethod("poll", &LinqNetwork::Poll),
            InstanceMethod("listen", &LinqNetwork::Listen),
            InstanceMethod("connect", &LinqNetwork::Connect),
            InstanceMethod("close", &LinqNetwork::Close),
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
        .on_new([this](const char* serial) {
            if (this->r_callback_) {
                // Create event data
                auto env = this->r_callback_.Env();
                auto sid = Napi::String::New(env, serial);

                // emit
                auto event = Napi::String::New(env, "new");
                this->r_callback_.Call({ event, sid });
            }
        })
        .on_heartbeat([this](const char* serial) {
            if (this->r_callback_) {
                // Create event data
                auto env = this->r_callback_.Env();
                auto sid = Napi::String::New(env, serial);

                // emit
                auto event = Napi::String::New(env, "heartbeat");
                this->r_callback_.Call({ event, sid });
            }
        })
        .on_alert([this](
                      const char* serial,
                      netw_alert_s* alert,
                      netw_email_s* email) {
            // Create event edata
            auto env = this->r_callback_.Env();
            Napi::Object obj = Napi::Object::New(env);
            Napi::Array arr = Napi::Array::New(env, 5);
            std::string who{ alert->who.p, alert->who.len };
            std::string what{ alert->what.p, alert->what.len };
            int when{ std::stoi({ alert->when.p, alert->when.len }) };
            std::string where{ alert->where.p, alert->where.len };
            std::string mesg{ alert->mesg.p, alert->mesg.len };
            std::string name{ alert->name.p, alert->name.len };
            std::string product{ alert->product.p, alert->product.len };
            std::string from{ email->from.p, email->from.len };
            std::string subject{ email->subject.p, email->subject.len };
            std::string user{ email->user.p, email->user.len };
            std::string password{ email->password.p, email->password.len };
            std::string server{ email->server.p, email->server.len };
            std::string port{ email->port.p, email->port.len };
            std::string device{ email->device.p, email->device.len };
            std::string sid{ serial };
            std::string to[] = { { email->to0.p, email->to0.len },
                                 { email->to1.p, email->to1.len },
                                 { email->to2.p, email->to2.len },
                                 { email->to3.p, email->to3.len },
                                 { email->to4.p, email->to4.len } };
            for (int i = 0; i < 5; i++) { arr[i] = to[i]; }
            obj.Set("who", who);
            obj.Set("what", what);
            obj.Set("where", where);
            obj.Set("when", when);
            obj.Set("mesg", mesg);
            obj.Set("name", name);
            obj.Set("product", product);
            obj.Set("serial", sid);
            obj.Set("from", from);
            obj.Set("subject", subject);
            obj.Set("user", user);
            obj.Set("password", password);
            obj.Set("server", server);
            obj.Set("port", port);
            obj.Set("device", device);
            obj.Set("to", arr);

            // emit
            auto event = Napi::String::New(env, "alert");
            this->r_callback_.Call({ event, obj });
        })
        .on_error([this](E_LINQ_ERROR e, const char* serial, const char* err) {
            ((void)serial);
            ((void)err);
            // Create event data
            auto env = this->r_callback_.Env();
            Napi::Object obj = Napi::Object::New(env);
            auto code = Napi::Number::New(env, e);
            std::string sid{ serial };
            std::string msg{ err };
            obj.Set("serial", sid);
            obj.Set("errorCode", code);
            obj.Set("errorMessage", msg);

            // emit
            auto event = Napi::String::New(env, "error");
            this->r_callback_.Call({ event, obj });
        })
        .on_ctrlc([this]() {
            // Create event data
            auto env = this->r_callback_.Env();

            // emit
            auto event = Napi::String::New(env, "ctrlc");
            this->r_callback_.Call({ event });

            // shutdown
            this->shutdown();
        });
}

LinqNetwork::~LinqNetwork() {}

Napi::Value
LinqNetwork::EarlyDestruct(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    this->linq_.early_destruct();
    return env.Null();
}

Napi::Value
LinqNetwork::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "0.0.1");
}

Napi::Value
LinqNetwork::RegisterCallback(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (!(info.Length() >= 1)) _NTHROW(env, "Incorrect number of arguments!");
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
    auto _s = this->linq_.listen(arg0.c_str());
    return info.This();
}

Napi::Value
LinqNetwork::Connect(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    // Validate inputs
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");

    // Call c routine with arguments
    std::string arg0 = info[0].ToString();
    auto _s = this->linq_.connect(arg0.c_str());
    return info.This();
}

Napi::Value
LinqNetwork::Close(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as Number!");
    uint32_t arg0 = info[0].ToNumber();
    this->linq_.close(arg0);
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
        err.Int32Value() == 0 ? deferred.Resolve(json) : deferred.Reject(err);
    });
    return deferred.Promise();
}

void
LinqNetwork::shutdown()
{
    this->shutdown_ = true;
}
