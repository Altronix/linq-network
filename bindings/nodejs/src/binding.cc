#ifdef BUILD_LINQD
#include "DaemonJS.h"
#endif

#ifdef BUILD_USBH
#include "UsbhJS.h"
#endif

#include "NetwJS.h"
#include <napi.h>

Napi::Object
Init(Napi::Env env, Napi::Object exports)
{
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("version", "0.0.1");
    obj.Set("network", LinqNetwork::Init(env, exports));

#ifdef BUILD_LINQD
    obj.Set("daemon", LinqDaemon::Init(env, exports));
#endif

#ifdef BUILD_USBH
    obj.Set("usbh", LinqUsbh::Init(env, exports));
#endif
    return obj;
}

NODE_API_MODULE(addon, Init)
