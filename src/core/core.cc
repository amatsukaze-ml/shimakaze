#include "core.h"

#include "v8/module.h"
#include "context/process_object.h"
#include "../hooks/cocos/scheduler_hook.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>

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

            // assert the shimakaze files exist
            assert_shimakaze_directory();

            // create a v8 isolate environment
            v8::Isolate::CreateParams create_params;
            create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate *isolate = v8::Isolate::New(create_params);
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);

            // create a main context for the isolate
            v8::Local<v8::Context> main_context = create_main_context(isolate);
            g_global_context.Reset(isolate, main_context);

            // retain the livelihood of the pointer and release it
            g_platform = platform.release();

            // run
            scheduler::run_under_context(isolate, run_main_context);
        }

        void assert_shimakaze_directory()
        {
            // get the base paths
            auto base_path = std::filesystem::current_path();
            auto shimakaze_path = base_path / "shimakaze";
            auto shimakaze_mods_path = shimakaze_path / "mods";
            auto shimakaze_config_path = shimakaze_path / "shimakaze.toml";

            // get default config
            toml::table default_config = SHIMAKAZE_DEFAULT_CONFIG;

            // assert the shimakaze directory exists
            if (!std::filesystem::is_directory(shimakaze_path))
            {
                // create it since it doesn't
                std::filesystem::create_directory(shimakaze_path);
            }

            // assert the mods directory exists
            if (!std::filesystem::is_directory(shimakaze_mods_path))
            {
                // create it since it also doesn't
                std::filesystem::create_directory(shimakaze_mods_path);
            }

            // assert the config file exists
            if (!std::filesystem::is_regular_file(shimakaze_config_path))
            {
                // create it since it also doesn't
                std::ofstream config_file(shimakaze_config_path.c_str());

                config_file << toml::toml_formatter{default_config};

                config_file.close();
            }

            // assert all default config values aren't missing in the saved config file
            std::ifstream config_file_save(shimakaze_config_path.c_str());

            if (!config_file_save.is_open())
            {
                // somehow problem exists xd
                config_file_save.close();

                // remove the file and make a new one
                std::filesystem::remove(shimakaze_config_path);
                std::ofstream config_file(shimakaze_config_path.c_str());

                config_file << toml::toml_formatter{default_config};

                config_file.close();
            }
            else
            {
                // we can modify the file
                // assert the config file has the correct keys
                bool changed = false;

                // read the file into a string stream
                std::stringstream buf;
                buf << config_file_save.rdbuf();

                toml::table conf;
                try
                {
                    // we have config file
                    conf = toml::parse(buf.str());

                    /// TOML VALIDATION SECTION START
                    // main table
                    toml::table *shimakaze_main = conf["main"].as_table();
                    toml::table *default_main = default_config["main"].as_table();

                    // assert main table exists
                    if (!shimakaze_main)
                    {
                        changed = true;
                        conf.insert("main", default_config["main"]);

                        shimakaze_main = conf["main"].as_table();
                    }

                    // validate main table has all keys
                    for (auto [key, value] : default_config["main"].ref<toml::table>())
                    {
                        if (!shimakaze_main->contains(key))
                        {
                            changed = true;
                            shimakaze_main->insert(key, value);
                        }
                    }

                    /// TOML VALIDATION SECTION END
                    // save to file if changed
                    if (changed == true)
                    {
                        std::filesystem::remove(shimakaze_config_path);
                        std::ofstream config_file(shimakaze_config_path.c_str());

                        config_file << toml::toml_formatter{conf};

                        config_file.close();
                    }
                }
                catch (const toml::parse_error &err)
                {
                    // damn.
                    // invalid file.
                    std::filesystem::remove(shimakaze_config_path);
                    std::ofstream config_file(shimakaze_config_path.c_str());

                    config_file << toml::toml_formatter{default_config};

                    config_file.close();
                }
            }
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

                v8::Local<v8::Script> script;

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
            bind::Module globals(isolate);

            // process;
            globals.submodule("process", context::create_process_object(isolate));

            return globals.get_template();
        }

        v8::Local<v8::Context> create_main_context(v8::Isolate *isolate)
        {
            // return the context
            return v8::Context::New(isolate, NULL, core::create_global_object(isolate));
        }
    }
}