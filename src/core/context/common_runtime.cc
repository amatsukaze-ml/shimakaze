#include "common_runtime.h"

namespace shimakaze
{
    namespace context
    {
        // runtime
        void common_include(const v8::FunctionCallbackInfo<v8::Value> &args) {
            // isolate
            v8::Isolate* isolate = args.GetIsolate();

            if (args.Length() != 1) {
                // only accept one argument
                isolate->ThrowException(v8::Exception::ReferenceError(bind::to_v8(isolate, "include() only takes one argument.")));
                return;
            }

            // get local script name
            // convert the first argument to a string
            v8::String::Utf8Value str(isolate, args[0]);
            v8::String::Utf8Value unscript_name(isolate, v8::StackTrace::CurrentScriptNameOrSourceURL(isolate));

            const char* script_name = bind::to_cstr(unscript_name);
            const char* module_name = bind::to_cstr(str);

            if (strcmp(module_name, script_name) == 0) {
                // cannot include self
                isolate->ThrowException(v8::Exception::ReferenceError(bind::to_v8(isolate, "get the fuck out of here with that shit")));
                return;
            }

            // try to find the module
            auto module = core::handler::g_mod_map.find(module_name);

            if (module == core::handler::g_mod_map.end()) {
                // module not found
                isolate->ThrowException(v8::Exception::ReferenceError(bind::to_v8(isolate, "This module was not found, or it wasn't loaded by the time it was imported.")));
                return;
            }

        }   

        // create runtime
        void create_common_runtime(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> global)
        {
            // include()
            global->Set(isolate, "require", v8::FunctionTemplate::New(isolate, &common_include));

            return;
        }
    }
}