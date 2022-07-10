#pragma once

#include "../pch.h"

namespace shimakaze::bind {
    v8::Local<v8::String> to_v8(v8::Isolate *isolate, const char *str);
    v8::Local<v8::String> to_v8(v8::Isolate *isolate, std::string str);

    v8::Local<v8::Number> to_v8(v8::Isolate *isolate, double value);
    v8::Local<v8::Number> to_v8(v8::Isolate *isolate, float value);
    v8::Local<v8::Number> to_v8(v8::Isolate *isolate, int value);
    v8::Local<v8::Number> to_v8(v8::Isolate *isolate, long value);

    const char* to_cstr(const v8::String::Utf8Value& value);
    std::string to_str(const v8::String::Utf8Value& value);
}