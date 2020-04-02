#ifdef BUILD_LINQD
#include "linq_daemon_js.h"
#endif
#include "linq_network_js.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("network", LinqNetwork::Init(env, exports));
#ifdef BUILD_LINQD
    obj.Set("daemon", LinqDaemon::Init(env, exports));
#endif
    return obj;
}

NODE_API_MODULE(addon, Init)
