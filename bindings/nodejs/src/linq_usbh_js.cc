#include "linq_usbh_js.h"
#include "napi.h"

Napi::FunctionReference LinqUsbh::constructor;

Napi::Object
LinqUsbh::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env,
        "LinqUsbh",
        {
            InstanceMethod("version", &LinqUsbh::Version),
            InstanceMethod("scan", &LinqUsbh::Scan),
            InstanceMethod("send", &LinqUsbh::Send),
        });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("LinqUsbh", func);
    return exports;
}

LinqUsbh::LinqUsbh(const Napi::CallbackInfo& info)
    : shutdown_{ false }
    , Napi::ObjectWrap<LinqUsbh>(info)
{
    linq_usbh_init(&this->usbh);
}

LinqUsbh::~LinqUsbh()
{
    linq_usbh_free(&this->usbh);
}

Napi::Value
LinqUsbh::Version(const Napi::CallbackInfo& info)
{
    return Napi::String::New(info.Env(), "0.0.1");
}

Napi::Value
LinqUsbh::Scan(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    if (!(info.Length() && info[0].IsNumber() && info[1].IsNumber())) {
        Napi::TypeError::New(env, "Bad args").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    } else {
        uint32_t vend = info[0].ToNumber(), prod = info[1].ToNumber();
        int ret = linq_usbh_scan(&this->usbh, vend, prod);
        return Napi::Number::New(env, ret);
    }
}

Napi::Value
LinqUsbh::Send(const Napi::CallbackInfo& info)
{
    Napi::Env env = Env();
    if (!(info.Length() >= 3 && //
          info[0].IsString() && //
          info[1].IsString() && //
          info[2].IsString())) {
        Napi::TypeError::New(env, "Bad args").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    std::string sid = info[0].ToString();
    std::string meth = info[1].ToString();
    std::string path = info[2].ToString();
    std::string data = (info.Length() >= 3 && info[3].IsString())
                           ? info[3].ToString()
                           : std::string{ "" };
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
    int ret = linq_usbh_send_http_request_sync(
        &this->usbh,
        sid.c_str(),
        meth.c_str(),
        path.c_str(),
        data.length() ? data.c_str() : NULL);
    auto err = Napi::Number::New(env, ret);
    if (!(ret < 0)) {
        char bytes[2048];
        uint16_t code;
        ret = linq_usbh_recv_http_response_sync(
            &this->usbh, sid.c_str(), &code, bytes, sizeof(bytes));
        if (!(ret < 0)) {
            Napi::Object obj = Napi::Object::New(env);
            std::string data{ bytes };
            obj.Set("code", code);
            obj.Set("data", data);
            deferred.Resolve(obj);
        }
    } else {
        deferred.Reject(err);
    }
    return deferred.Promise();
}
