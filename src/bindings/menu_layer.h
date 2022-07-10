#pragma once

#include "../pch.h"

#include "../core/handler.h"
#include "../hooks/menu_layer.h"

namespace shimakaze::bindings
{
    v8::Local<v8::FunctionTemplate> cast_menulayer(v8::Isolate *isolate, CCLayer* self);

    v8::Local<v8::FunctionTemplate> bind_menulayer(v8::Isolate *isolate);
    void add_menulayer_hooks(v8::Local<v8::Context> context, v8::Local<v8::Function> menu_layer, v8::Local<v8::Function> function);
}