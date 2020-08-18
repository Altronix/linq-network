#include "NetwJS.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    // Napi::Object obj = Napi::Object::New(env);
    // obj.Set("version", "0.0.1");
    // obj.Set("network", LinqNetwork::Init(env, exports));
    // return obj;
    return LinqNetwork::Init(env, exports);
}

NODE_API_MODULE(addon, Init)
