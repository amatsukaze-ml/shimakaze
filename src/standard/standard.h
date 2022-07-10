#pragma once

#include "../pch.h"

namespace shimakaze::standard
{
    v8::Local<v8::Module> create_standard_module(v8::Isolate *isolate);
    void standard_hook(const v8::FunctionCallbackInfo<v8::Value> &args);
}