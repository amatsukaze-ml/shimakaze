#include "runtime.h"

namespace shimakaze::context
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

    char* get_mod_name(const v8::FunctionCallbackInfo<v8::Value> &args) {
        v8::Isolate* isolate = args.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::String::Utf8Value mod_name(isolate, v8::Local<v8::String>::Cast(context->GetEmbedderData(0)));

        return *mod_name;
    }

    std::string get_logger_string(const v8::FunctionCallbackInfo<v8::Value> &args) {
        v8::Isolate* isolate = args.GetIsolate();

        bool isFirst = true;
        std::string result = "";

        for (int i = 0; i < args.Length(); ++i) {
            if (isFirst)
                isFirst = false;
            else
                result += " ";

            v8::String::Utf8Value arg(isolate, v8::Local<v8::String>::Cast(args[i]));
            result += bind::to_str(arg);
        }
        return result;
    }

    void logger_debug(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::debug(get_mod_name(args), get_logger_string(args).c_str());
    }

    void logger_info(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::info(get_mod_name(args), get_logger_string(args).c_str());
    }

    void logger_warn(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::warn(get_mod_name(args), get_logger_string(args).c_str());
    }

    void logger_error(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::error(get_mod_name(args), get_logger_string(args).c_str());
    }

    void logger_fatal(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::fatal(get_mod_name(args), get_logger_string(args).c_str());
    }

    void logger_severe(const v8::FunctionCallbackInfo<v8::Value> &args) {
        shimakaze::console::severe(get_mod_name(args), get_logger_string(args).c_str());
    }

    // create runtime
    void create_common_runtime(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> global)
    {
        // include()
        global->Set(isolate, "require", v8::FunctionTemplate::New(isolate, &common_include));

        // create console object
        auto console_object = v8::ObjectTemplate::New(isolate);
        console_object->Set(isolate, "debug", v8::FunctionTemplate::New(isolate, &logger_debug));
        console_object->Set(isolate, "info", v8::FunctionTemplate::New(isolate, &logger_info));
        console_object->Set(isolate, "warn", v8::FunctionTemplate::New(isolate, &logger_warn));
        console_object->Set(isolate, "error", v8::FunctionTemplate::New(isolate, &logger_error));
        console_object->Set(isolate, "fatal", v8::FunctionTemplate::New(isolate, &logger_fatal));
        console_object->Set(isolate, "severe", v8::FunctionTemplate::New(isolate, &logger_severe));

        global->Set(isolate, "logger", console_object);

        return;
    }
}