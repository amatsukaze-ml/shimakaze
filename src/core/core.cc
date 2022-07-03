#include "core.h"

#include "context/process_object.h"
#include "../hooks/cocos/scheduler_hook.h"

namespace shimakaze
{
    namespace core
    {
        void start()
        {
            // create a platform for v8
            std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
            v8::V8::InitializePlatform(platform.get());

            // initialize v8
            v8::V8::Initialize();

            // create a v8 isolate environment
            v8::Isolate::CreateParams create_params;
            create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate *isolate = v8::Isolate::New(create_params);
            std::cout << "created isolate" << std::endl;
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);

            std::cout << "created isolate SCOPE" << std::endl;

            // create a main context for the isolate
            v8::Local<v8::Context> main_context = create_main_context(isolate);
            std::cout << "created context" << std::endl;

            g_global_context.Reset(isolate, main_context);
            
            // retain the livelihood of the pointer and release it
            g_platform = platform.release(); 

            // run the context
            std::cout << "created run context func" << std::endl;
            scheduler::run_under_context(isolate, run_main_context);
            std::cout << "successfully added runner" << std::endl;
        }

        void run_main_context(v8::Local<v8::Context> context)
        {
            std::cout << "running context" << std::endl;
            
            // get the main isolate
            v8::Isolate *main_isolate = context->GetIsolate();
            std::cout << "creating context scope" << std::endl;
            v8::Context::Scope context_scope(context);
            {
                // our scope!
                v8::HandleScope handle_scope(main_isolate);
                std::cout << "context scope created" << std::endl;

                std::cout << "running script" << std::endl;
                v8::Local<v8::String> name(v8::String::NewFromUtf8Literal(context->GetIsolate(), "(shimakaze)"));

                v8::TryCatch try_catch(main_isolate);
                v8::Local<v8::String> source = v8::String::NewFromUtf8(context->GetIsolate(), "\"hi\"", v8::NewStringType::kNormal).ToLocalChecked();

                std::cout << "created script name and scope" << std::endl;
                v8::Local<v8::Script> script;
                std::cout << "created script object" << std::endl;

                bool compiled = v8::Script::Compile(context, source).ToLocal(&script);
                if (!compiled)
                {
                    std::cout << "didnt compile :(" << std::endl;
                    std::cout << *try_catch.Exception() << std::endl;
                }
                else
                {
                    std::cout << "compiled script" << std::endl;
                    v8::Local<v8::Value> result;
                    if (!script->Run(context).ToLocal(&result))
                    {
                        if (try_catch.HasCaught())
                        {
                            std::cout << *try_catch.Exception() << std::endl;
                            return;
                        }
                    }
                    else
                    {
                        std::cout << "ran script" << std::endl;
                        if (try_catch.HasCaught())
                        {
                            std::cout << *try_catch.Exception() << std::endl;
                            return;
                        }

                        if (!result->IsUndefined())
                        {
                            v8::String::Utf8Value str(main_isolate, result);
                            const char *cstr = *str;
                            printf("%s\n", cstr);
                        }
                    }
                }
            }
        }

        v8::Local<v8::ObjectTemplate> create_global_object(v8::Isolate *isolate)
        {
            // create a object template for the context's globals
            v8::Local<v8::ObjectTemplate> globals = v8::ObjectTemplate::New(isolate);

            // process;
            globals->Set(isolate,
                         "process",
                         context::create_process_object(isolate));

            return globals;
        }

        v8::Local<v8::Context> create_main_context(v8::Isolate *isolate)
        {
            // return the context
            return v8::Context::New(isolate, NULL, core::create_global_object(isolate));
        }
    }
}