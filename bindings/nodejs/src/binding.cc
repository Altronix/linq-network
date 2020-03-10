#include "linq_network_js.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("network", LinqNetwork::Init(env, exports));
    return obj;
}

NODE_API_MODULE(addon, Init)
