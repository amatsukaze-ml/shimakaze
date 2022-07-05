#pragma once

#include "../../pch.h"

namespace shimakaze
{
    namespace scheduler
    {
        void run_under_context(v8::Isolate* isolate, std::string script, std::function<void(std::string, v8::Local<v8::Context>)> func);
        void run_value_under_context(v8::Isolate* isolate, v8::Local<v8::Value> value, std::function<void(v8::Local<v8::Value>, v8::Local<v8::Context>)> func);
        
        void run_context_function(v8::Isolate* isolate, std::string script,  std::function<void(std::string, v8::Local<v8::Context>)> func);
        void run_value_context_function(v8::Isolate* isolate, v8::Local<v8::Value> value, std::function<void(v8::Local<v8::Value>, v8::Local<v8::Context>)> func);
    
        inline bool in_context_runner = true;
        inline bool scheduler_started = false;
    }
}

SHIMAKAZE_HOOK_ARGS(CCScheduler_update, CCScheduler *, void, float dt);

