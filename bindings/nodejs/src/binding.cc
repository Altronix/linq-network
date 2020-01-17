#include "atxnet.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    return LinqNetwork::Init(env, exports);
}

NODE_API_MODULE(addon, Init)
