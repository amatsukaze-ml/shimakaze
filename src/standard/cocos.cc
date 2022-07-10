#include "cocos.h"

#include "../bindings/cocos/ccsprite.h"

namespace shimakaze::standard
{
    v8::Local<v8::Module> create_cocos_module(v8::Isolate *isolate)
    {

        return v8::Module::CreateSyntheticModule(
            isolate,
            bind::to_v8(isolate, "cocos2d"),
            {
                bind::to_v8(isolate, "default"), // cringe
                bind::to_v8(isolate, "CCSprite"),
            },
            [](v8::Local<v8::Context> context, v8::Local<v8::Module> module)
            {
                v8::Isolate *isolate = context->GetIsolate();

                // export default
                module->SetSyntheticModuleExport(
                    isolate,
                    bind::to_v8(isolate, "default"),
                    v8::Null(isolate));

                // cocos2d.CCSprite
                module->SetSyntheticModuleExport(
                    isolate,
                    bind::to_v8(isolate, "CCSprite"),
                    bindings::bind_ccsprite(isolate)->GetFunction(context).ToLocalChecked());

                return v8::MaybeLocal<v8::Value>(v8::True(isolate));
            });
    }
}