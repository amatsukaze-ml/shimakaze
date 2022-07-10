#include "../../pch.h"

#include "../core.h"
#include "../handler.h"

namespace shimakaze::context
{
    // runtime
    void common_include(const v8::FunctionCallbackInfo<v8::Value> &args);

    // create runtime
    void create_common_runtime(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> global);

    // logger
    void logger_debug(const v8::FunctionCallbackInfo<v8::Value> &args);
    void logger_info(const v8::FunctionCallbackInfo<v8::Value> &args);
    void logger_warn(const v8::FunctionCallbackInfo<v8::Value> &args);
    void logger_error(const v8::FunctionCallbackInfo<v8::Value> &args);
    void logger_fatal(const v8::FunctionCallbackInfo<v8::Value> &args);
    void logger_severe(const v8::FunctionCallbackInfo<v8::Value> &args);

    // logger utilities
    std::string get_logger_string(const v8::FunctionCallbackInfo<v8::Value> &args);
    char* get_mod_name(const v8::FunctionCallbackInfo<v8::Value> &args);
}