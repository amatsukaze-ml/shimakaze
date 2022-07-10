#include "gd.h"

#include "../bindings/menu_layer.h"

namespace shimakaze::standard
{
    v8::Local<v8::Module> create_gd_module(v8::Isolate *isolate) {
        return v8::Module::CreateSyntheticModule(
                isolate,
                bind::to_v8(isolate, "gd"),
                {
                        bind::to_v8(isolate, "default"), // cringe
                        bind::to_v8(isolate, "MenuLayer"),
                },
                [](v8::Local<v8::Context> context, v8::Local<v8::Module> module)
                {
                    v8::Isolate *isolate = context->GetIsolate();

                    // export default
                    module->SetSyntheticModuleExport(
                            isolate,
                            bind::to_v8(isolate, "default"),
                            v8::Null(isolate));

                    // gd.MenuLayer
                    module->SetSyntheticModuleExport(
                            isolate,
                            bind::to_v8(isolate, "MenuLayer"),
                            bindings::bind_menulayer(isolate)->GetFunction(context).ToLocalChecked());

                    return v8::MaybeLocal<v8::Value>(v8::True(isolate));
                });
    }
}