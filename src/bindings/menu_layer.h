#include "../pch.h"

namespace shimakaze
{
    namespace bindings
    {
        void menulayer_init(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            // isolate
            v8::Isolate *isolate = args.GetIsolate();
        }

        void fuck_you(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            // no
        }

        v8::Local<v8::FunctionTemplate> bind_menulayer(v8::Isolate *isolate)
        {
            // create the menulayer
            v8::Local<v8::FunctionTemplate> menulayer_ctor = v8::FunctionTemplate::New(isolate, fuck_you);

            // set shit
            menulayer_ctor->Set(
                bind::to_v8(isolate, "init"),
                v8::FunctionTemplate::New(isolate, menulayer_init));

            return menulayer_ctor;
        }
    }
}