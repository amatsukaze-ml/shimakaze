#pragma once

#include "../../pch.h"

namespace shimakaze
{
    namespace scheduler
    {
        void run_under_context(v8::Isolate* isolate, std::function<void(v8::Local<v8::Context>)> func);
        void run_context_function(v8::Isolate* isolate, std::function<void(v8::Local<v8::Context>)> func);
    }
}

SHIMAKAZE_HOOK_ARGS(CCScheduler_update, CCScheduler *, void, float dt);

