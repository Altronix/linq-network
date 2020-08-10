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
