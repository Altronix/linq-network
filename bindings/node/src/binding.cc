#include "LogJS.h"
#include "LinqJS.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    Logger::Init(env, exports);
    Linq::Init(env, exports);
    return exports;
}

NODE_API_MODULE(addon, Init)
