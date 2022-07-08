#include "convert.h"

namespace shimakaze
{
    namespace bind
    {
        v8::Local<v8::String> to_v8(v8::Isolate *isolate, const char *str)
        {
            return v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal).ToLocalChecked();
        }

        v8::Local<v8::String> to_v8(v8::Isolate *isolate, std::string str)
        {
            return v8::String::NewFromUtf8(isolate, str.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
        }

        v8::Local<v8::Number> to_v8(v8::Isolate *isolate, double value)
        {
            return v8::Number::New(isolate, value);
        }

        v8::Local<v8::Number> to_v8(v8::Isolate *isolate, float value)
        {
            return v8::Number::New(isolate, value);
        }

        v8::Local<v8::Number> to_v8(v8::Isolate *isolate, int value)
        {
            return v8::Number::New(isolate, value);
        }

        v8::Local<v8::Number> to_v8(v8::Isolate *isolate, long value)
        {
            return v8::Number::New(isolate, value);
        }

        const char* to_cstr(const v8::String::Utf8Value& str)
        {
            return *str ? *str : "<unknown string>";
        }

        std::string to_str(const v8::String::Utf8Value& str)
        {
            return *str ? std::string(*str) : "<unknown string>";
        }
    }
}