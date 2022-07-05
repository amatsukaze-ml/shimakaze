#include "../../pch.h"

#include "../v8/module.h"

namespace shimakaze {
    namespace context {
        // process functions
        void process_print(const v8::FunctionCallbackInfo<v8::Value>& args);

        // create process object
        v8::Local<v8::ObjectTemplate> create_process_object(v8::Isolate* isolate);
    }
}