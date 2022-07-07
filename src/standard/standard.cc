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
                    bind::to_v8(isolate, "default"), // cringe
                    bind::to_v8(isolate, "hook"),
                    bind::to_v8(isolate, "Version")
                },
                [](v8::Local<v8::Context> context, v8::Local<v8::Module> module)
                {
                    v8::Isolate *isolate = context->GetIsolate();

                    // export default
                    module->SetSyntheticModuleExport(
                        isolate,
                        bind::to_v8(isolate, "default"),
                        v8::Null(isolate));

                    // shimakaze.hook();
                    module->SetSyntheticModuleExport(
                        isolate,
                        bind::to_v8(isolate, "hook"),
                        v8::FunctionTemplate::New(isolate, standard_hook)->GetFunction(context).ToLocalChecked());

                    // shimakaze.Version
                    module->SetSyntheticModuleExport(
                        isolate,
                        bind::to_v8(isolate, "Version"),
                        bind::to_v8(isolate, SHIMAKAZE_VERSION)
                    );

                    return v8::MaybeLocal<v8::Value>(v8::True(isolate));
                });
        }

        void standard_hook(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            v8::Isolate *isolate = args.GetIsolate();
            if (args[0]->IsFunction()) {
                std::cout << "hi" << std::endl;
                v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(args[0]);

                v8::String::Utf8Value funcname(isolate, func->GetInferredName());

                std::cout << bind::to_cstr(funcname) << std::endl;
            }
        }
    }
}