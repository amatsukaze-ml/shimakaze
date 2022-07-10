#include "../../pch.h"

#include "../core.h"
#include "../handler.h"

namespace shimakaze::context
{
    // runtime
    void common_include(const v8::FunctionCallbackInfo<v8::Value> &args);

    // create runtime
    void create_common_runtime(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> global);
}