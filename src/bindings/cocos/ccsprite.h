#include "../pch.h"

namespace shimakaze {
    namespace bindings {
        void ccsprite_create(const v8::FunctionCallbackInfo<v8::Value>& args) {
            // isolate
            v8::Isolate* isolate = args.GetIsolate();

            // get the file name
            v8::String::Utf8Value file_name(isolate, args[0]);

            // create a ccsprite
            CCSprite* sprite = CCSprite::create(bind::to_cstr(file_name));

            // set this
            args.This()->SetInternalField(0, v8::External::New(isolate, sprite));
        }
        
        v8::Local<v8::FunctionTemplate> bind_ccsprite(v8::Isolate* isolate) {
            // create the ccsprite class
            v8::Local<v8::FunctionTemplate> ccsprite_ctor = v8::FunctionTemplate::New(isolate, ccsprite_create);

            // set class details
            ccsprite_ctor->SetClassName(bind::to_v8(isolate, "CCSprite"));
            ccsprite_ctor->InstanceTemplate()->SetInternalFieldCount(1); // space for the pointer

            return ccsprite_ctor;
        }
    }
}