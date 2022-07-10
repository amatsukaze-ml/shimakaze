#pragma once

#include "../pch.h"

namespace shimakaze::bindings
{
    void ccsprite_create(const v8::FunctionCallbackInfo<v8::Value> &args)
    {
        // isolate
        v8::Isolate *isolate = args.GetIsolate();

        // get the file name
        v8::String::Utf8Value file_name(isolate, args[0]);

        // create a ccsprite
        CCSprite *sprite = CCSprite::create(bind::to_cstr(file_name));

        // set this
        args.This()->SetInternalField(0, v8::External::New(isolate, sprite));
        args.GetReturnValue().Set(args.This());
    }

    v8::Local<v8::FunctionTemplate> bind_ccsprite(v8::Isolate *isolate)
    {
        // create the ccsprite class
        v8::Local<v8::FunctionTemplate> ccsprite_ctor = v8::FunctionTemplate::New(isolate, ccsprite_create);

        // set class details
        ccsprite_ctor->SetClassName(bind::to_v8(isolate, "CCSprite"));
        ccsprite_ctor->InstanceTemplate()->SetInternalFieldCount(1); // space for the pointer

        // CCSprite::setPosition()
        ccsprite_ctor->PrototypeTemplate()->Set(
            bind::to_v8(isolate, "setPosition"),
            v8::FunctionTemplate::New(
                isolate,
                [](const v8::FunctionCallbackInfo<v8::Value> &args)
                {
                    // isolate
                    v8::Isolate *isolate = args.GetIsolate();

                    // get self
                    v8::Local<v8::Object> self_obj = args.This();
                    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self_obj->GetInternalField(0));

                    // cast to self
                    CCSprite *self = static_cast<CCSprite *>(wrap->Value());

                    if (args.Length() == 2)
                    {
                        // get x and y
                        double x = args[0]->NumberValue(isolate->GetCurrentContext()).FromJust();
                        double y = args[1]->NumberValue(isolate->GetCurrentContext()).FromJust();

                        // set position
                        self->setPosition(ccp(x, y));
                    }
                    else
                    {
                        // error
                        isolate->ThrowException(bind::to_v8(isolate, "Invalid arguments"));
                    }
                }));

        return ccsprite_ctor;
    }
}