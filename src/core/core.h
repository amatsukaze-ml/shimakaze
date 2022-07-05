#pragma once

#include "../pch.h"

#include <map>
#include <deque>

namespace shimakaze {
    namespace core {
        inline v8::Global<v8::Context> g_global_context;
        inline extern v8::Platform* g_platform = nullptr;
        inline toml::table g_config;
        inline std::deque<toml::table> g_mod_info;
        
        void start();
        void dispose(v8::Isolate* isolate);
        void assert_shimakaze_directory();

        void run_under_main_context(std::string script, v8::Local<v8::Context> context);
        
        v8::Local<v8::ObjectTemplate> create_global_object(v8::Isolate *isolate);
        v8::Local<v8::Context> create_main_context(v8::Isolate *isolate);

    }
}