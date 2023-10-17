#include "LinqJS.h"
#include "napi.h"
#include <chrono>

#define _NTHROW(__env, __err)                                                  \
    do {                                                                       \
        Napi::TypeError::New(__env, __err).ThrowAsJavaScriptException();       \
    } while (0)

Napi::FunctionReference Linq::constructor;

Napi::Object
Linq::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "Linq",
        { InstanceMethod("earlyDestruct", &Linq::EarlyDestruct),
          InstanceMethod("version", &Linq::Version),
          InstanceMethod("registerCallback", &Linq::RegisterCallback),
          InstanceMethod("isRunning", &Linq::IsRunning),
          InstanceMethod("poll", &Linq::Poll),
          InstanceMethod("root", &Linq::Root),
          InstanceMethod("listen", &Linq::Listen),
          InstanceMethod("connect", &Linq::Connect),
          InstanceMethod("close", &Linq::Close),
          InstanceMethod("devices", &Linq::Devices),
          InstanceMethod("deviceCount", &Linq::DeviceCount),
          InstanceMethod("deviceRemove", &Linq::DeviceRemove),
          InstanceMethod("nodeCount", &Linq::NodeCount),
          InstanceMethod("send", &Linq::Send),
          InstanceMethod("sendGet", &Linq::Get),
          InstanceMethod("sendPost", &Linq::Post),
          InstanceMethod("sendDelete", &Linq::Del),
          InstanceMethod("scan", &Linq::Scan) });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Linq", func);
    return exports;
}

Linq::Linq(const Napi::CallbackInfo& info)
    : shutdown_{ false }
    , Napi::ObjectWrap<Linq>(info)
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
        .on_alert(
            [this](
                const char* serial, netw_alert_s* alert, netw_email_s* email) {
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
            // Create event data
            auto env = this->r_callback_.Env();
            Napi::Object obj = Napi::Object::New(env);
            auto code = Napi::Number::New(env, e);
            std::string sid{ serial };
            std::string msg{ err };
            obj.Set("errorCode", code);
            obj.Set("errorMessage", msg);
            if (sid.length()) obj.Set("serial", sid);

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

Linq::~Linq() {}

Napi::Value
Linq::EarlyDestruct(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    this->linq_.early_destruct();
    return env.Null();
}

Napi::Value
Linq::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), this->linq_.version());
}

Napi::Value
Linq::RegisterCallback(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();

    if (!(info.Length() >= 1)) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsFunction())) _NTHROW(env, "Expect arg[0] as Function!");
    this->r_callback_ = Napi::Persistent(info[0].As<Napi::Function>());
    return env.Null();
}

Napi::Value
Linq::IsRunning(const Napi::CallbackInfo& info)
{
    return Napi::Boolean::New(info.Env(), !this->shutdown_);
}

Napi::Value
Linq::Poll(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as String!");
    uint32_t ms = info[0].ToNumber();
    this->linq_.poll(ms);
    return info.Env().Null();
}

Napi::Value
Linq::Root(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");
    std::string root = info[0].ToString();
    this->linq_.root(root.c_str());
    return info.This();
}

Napi::Value
Linq::Listen(const Napi::CallbackInfo& info)
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
Linq::Connect(const Napi::CallbackInfo& info)
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
Linq::Close(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length())) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsNumber())) _NTHROW(env, "Expect arg[0] as Number!");
    uint32_t arg0 = info[0].ToNumber();
    this->linq_.close(arg0);
    return info.Env().Null();
}

Napi::Value
Linq::Devices(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), this->linq_.devices());
}

Napi::Value
Linq::DeviceCount(const Napi::CallbackInfo& info)
{
    return Napi::Number::New(info.Env(), this->linq_.device_count());
}

Napi::Value
Linq::NodeCount(const Napi::CallbackInfo& info)
{
    return Napi::Number::New(info.Env(), this->linq_.node_count());
}

Napi::Value
Linq::DeviceRemove(const Napi::CallbackInfo& info)
{
    Napi::Env env = Env();
    if (!(info.Length() >= 1)) _NTHROW(env, "Incorrect number of arguments!");
    if (!(info[0].IsString())) _NTHROW(env, "Expect arg[0] as String!");
    std::string sid = info[0].ToString();
    return Napi::Number::New(info.Env(), this->linq_.device_remove(sid));
}

Napi::Value
Linq::Get(const Napi::CallbackInfo& info)
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
Linq::Post(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
Linq::Del(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "TODO");
}

Napi::Value
Linq::Send(const Napi::CallbackInfo& info)
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
        (err.Int32Value() == 0 || err.Int32Value() == 200)
            ? deferred.Resolve(json)
            : deferred.Reject(err);
    });
    return deferred.Promise();
}

void
Linq::shutdown()
{
    this->shutdown_ = true;
}

Napi::Value
Linq::Scan(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    int ret = this->linq_.scan();
    return Napi::Number::New(env, ret);
}
