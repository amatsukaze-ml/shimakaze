#include "menu_layer.h"

#include "../hooks/cocos/scheduler_hook.h"

namespace shimakaze::bindings
{
    CCLayer *bruh;
    v8::Local<v8::FunctionTemplate> cast_menulayer(v8::Isolate *isolate, CCLayer *self)
    {
        // create the menulayer
        v8::Local<v8::FunctionTemplate> menulayer_ctor = v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &args) {});

        // set class name
        menulayer_ctor->SetClassName(bind::to_v8(isolate, "MenuLayer"));

        bruh = self;

        // set functions
        menulayer_ctor->Set(
            bind::to_v8(isolate, "addChild"),
            v8::FunctionTemplate::New(
                isolate,
                [](const v8::FunctionCallbackInfo<v8::Value> &args)
                {
                    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(args[0]);

                    // voidptr
                    if (obj->InternalFieldCount() > 0)
                    {
                        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(obj->GetInternalField(0));
                        void *obj_ptr = wrap->Value();

                        // add the child
                        CCSprite* child = static_cast<CCSprite *>(obj_ptr);
                        bruh->addChild(child);
                    }
                }));

        return menulayer_ctor;
    }

    v8::Local<v8::FunctionTemplate> bind_menulayer(v8::Isolate *isolate)
    {
        // create the menulayer
        v8::Local<v8::FunctionTemplate> menulayer_ctor = v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &args) {});

        // set class name
        menulayer_ctor->SetClassName(bind::to_v8(isolate, "MenuLayer"));

        return menulayer_ctor;
    }

    void add_menulayer_hooks(v8::Local<v8::Context> context, v8::Local<v8::Function> menu_layer, v8::Local<v8::Function> function)
    {
        // check functions
        if (!function->Get(context, bind::to_v8(context->GetIsolate(), "init")).IsEmpty())
        {
            // reference the function
            v8::Local<v8::Function> init_func = v8::Local<v8::Function>::Cast(function->Get(context, bind::to_v8(context->GetIsolate(), "init")).ToLocalChecked());

            // add hook
            scheduler::add_hook_on_main_thread("menulayer", "init", COPYABLE_PERSISTENT<v8::Function>(context->GetIsolate(), init_func));
        }
    }
}