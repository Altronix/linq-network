#include "log.h"
#include <napi.h>

class Logger : public Napi::ObjectWrap<Logger>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Logger(const Napi::CallbackInfo& info);
    ~Logger();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value Open(const Napi::CallbackInfo& info);
    Napi::Value Close(const Napi::CallbackInfo& info);
    Napi::FunctionReference callback;
    static void logs(log_callback_s* callback)
    {
        Logger* l = (Logger*)callback->context;
        if (l->callback) {
            auto env = l->callback.Env();
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("tick", Napi::Number::New(env, callback->tick));
            obj.Set("line", Napi::Number::New(env, callback->line));
            obj.Set("file", Napi::String::New(env, callback->file));
            obj.Set("level", Napi::String::New(env, callback->level));
            obj.Set("mesg", Napi::String::New(env, callback->message));
            l->callback.Call({ obj });
        }
    }
};
