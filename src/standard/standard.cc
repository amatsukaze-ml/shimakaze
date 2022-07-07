#include "standard.h"

namespace shimakaze
{
    namespace standard
    {
        v8::Local<v8::Module> create_standard_module(v8::Isolate *isolate)
        {
            return v8::Module::CreateSyntheticModule(
                isolate,
                bind::to_v8(isolate, "shimakaze"),
                {
                    bind::to_v8(isolate, "hook")
                },
                [](v8::Local<v8::Context> context, v8::Local<v8::Module> module)
                {
                    v8::Isolate *isolate = context->GetIsolate();

                    // shimakaze.hook();
                    module->SetSyntheticModuleExport(
                        isolate,
                        bind::to_v8(isolate, "hook"),
                        v8::FunctionTemplate::New(isolate, standard_hook)->GetFunction(context).ToLocalChecked());

                    return v8::MaybeLocal<v8::Value>(v8::True(isolate));
                });
        }

        void standard_hook(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
        }
    }
}