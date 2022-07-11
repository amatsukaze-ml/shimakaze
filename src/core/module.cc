#include "module.h"

#include "handler.h"

#include <filesystem>

namespace shimakaze::module
{
    v8::MaybeLocal<v8::Promise> dynamic_call(v8::Local<v8::Context> context, v8::Local<v8::Data> host_defined_options, v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions)
    {
        // create a promise for rejection and resolution
        v8::Local<v8::Promise::Resolver> promise_resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
        v8::MaybeLocal<v8::Promise> promise(promise_resolver->GetPromise());

        // get the specifier name
        v8::String::Utf8Value specifier_name(context->GetIsolate(), specifier);

        promise_resolver->Resolve(context, specifier);
        return promise;
    }

    v8::MaybeLocal<v8::Module> static_call(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions, v8::Local<v8::Module> referrer)
    {
        v8::String::Utf8Value specifier_name(context->GetIsolate(), specifier);
        const char *specifier_name_cstr = bind::to_cstr(specifier_name);

        // check standard libraries first
        auto standard_call = core::handler::g_libraries.find(specifier_name_cstr);
        if (standard_call != core::handler::g_libraries.end())
        {
            // this is a standard library
            // convert it to a local
            v8::Local<v8::Module> local_call = v8::Local<v8::Module>::New(context->GetIsolate(), standard_call->second);

            // return it as a maybe local
            return v8::MaybeLocal<v8::Module>(local_call);
        }

        // check path
        // and normalize it
        auto mod_id = context->GetEmbedderData(0).As<v8::String>();
        auto path = context->GetEmbedderData(1).As<v8::String>();
        v8::String::Utf8Value path_vstr(context->GetIsolate(), path);
        auto path_str = bind::to_str(path_vstr);
        auto main_path = (std::filesystem::path(path_str) / specifier_name_cstr).lexically_normal();

        if (std::filesystem::is_regular_file(main_path))
        {
            // read file
            std::ifstream file(main_path);

            if (!file.is_open())
            {
                // file could not be opened
                context->GetIsolate()->ThrowException(v8::Exception::ReferenceError(bind::to_v8(context->GetIsolate(), "This library could not be found.")));

                return v8::MaybeLocal<v8::Module>();
            }

            // read file contents to a string strream
            std::stringstream file_stream;
            file_stream << file.rdbuf();

            std::cout << "run" << std::endl;

            // try to compile the file
            // get the current isolate from the scope
            v8::Isolate *isolate = context->GetIsolate();

            // create a script name and origin
            v8::ScriptOrigin origin(isolate,
                                    mod_id,                 // resource name
                                    0,                      // resource line offset
                                    0,                      // resource column offset
                                    false,                  // resource is shared
                                    -1,                     // script id
                                    v8::Local<v8::Value>(), // source map URL
                                    false,                  // resource is opaque
                                    false,                  // is wasm
                                    true);                  // is module

            // turn the script's code into a v8 local
            v8::Local<v8::String> mod_code = bind::to_v8(isolate, file_stream.str());

            // create module data
            v8::MaybeLocal<v8::Module> mod_module;
            v8::ScriptCompiler::Source source(mod_code, origin);

            // compile the mod
            return v8::ScriptCompiler::CompileModule(isolate, &source);
        }
        else
        {
            // file could not be opened
            context->GetIsolate()->ThrowException(v8::Exception::ReferenceError(bind::to_v8(context->GetIsolate(), "This library could not be found.")));

            return v8::MaybeLocal<v8::Module>();
        }
    }

    void import_meta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta)
    {
        // do nothing for now
        meta->Set(
            context,
            bind::to_v8(context->GetIsolate(), "url"),
            bind::to_v8(context->GetIsolate(), "http://example.com/"));
    }
}