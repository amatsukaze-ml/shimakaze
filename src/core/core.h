#include "../pch.h"

namespace shimakaze {
    namespace core {
        void start();
        void run_main_context(v8::Local<v8::Context> context);
        v8::Local<v8::ObjectTemplate> create_global_object(v8::Isolate *isolate);
        v8::Local<v8::Context> create_main_context(v8::Isolate *isolate);

        inline v8::Global<v8::Context> g_global_context;
        inline extern v8::Platform* g_platform = nullptr;
    }
}