#include "standard.h"

#include "../bindings/menu_layer.h"

namespace shimakaze::standard
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

        // validate that the first and second argument is a function
        if (args[0]->IsFunction() && args[1]->IsFunction()) {
            v8::Local<v8::Function> our_hook = v8::Local<v8::Function>::Cast(args[0]);
            v8::Local<v8::Function> class_hook = v8::Local<v8::Function>::Cast(args[1]);

            v8::String::Utf8Value funcname(isolate, our_hook->GetName());

            // c++
            // why no string switch case :c
            if (bind::to_str(funcname) == "MenuLayer") {
                // menu layer bindings
                bindings::add_menulayer_hooks(isolate->GetCurrentContext(), our_hook, class_hook);
            }
        } else {
            std::cout << "Invalid arguments" << std::endl;
        }
    }
}