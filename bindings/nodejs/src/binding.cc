#include "LogJS.h"
#include "NetwJS.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    LinqNetwork::Init(env, exports);
    Logger::Init(env, exports);
    return exports;
}

NODE_API_MODULE(addon, Init)
