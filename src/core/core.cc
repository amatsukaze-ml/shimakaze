#include "core.h"
#include "handler.h"
#include "module.h"

#include "context/process.h"
#include "context/runtime.h"

#include "../standard/standard.h"
#include "../standard/cocos.h"
#include "../standard/gd.h"

#include "../hooks/loading_layer.h"
#include "../hooks/cocos/scheduler_hook.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <format>
#include <thread>

const auto base_path = std::filesystem::current_path();
const auto shimakaze_path = base_path / "shimakaze";
const auto shimakaze_mods_path = shimakaze_path / "mods";
const auto shimakaze_config_path = shimakaze_path / "shimakaze.toml";

// temp folders
const auto shimakaze_temp_path = shimakaze_path / ".temp";
const auto shimakaze_temp_images_path = shimakaze_temp_path / "images";

namespace shimakaze::core
{
    void start()
    {
        if (g_started) {
            // get texture & sprite frame cache
            auto texture_cache = CCTextureCache::sharedTextureCache();
            auto sprite_frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();

            // load textures
            texture_cache->addImage("shimakaze-GameSheet01.png", false);
            sprite_frame_cache->addSpriteFramesWithFile("shimakaze-GameSheet01.plist");

            // Just refresh the texture resolutions to whatever the user picked.
            for (const auto &mod : core::handler::g_mod_map)
            {
                auto entry = mod.second;
                entry->refresh_resources();
            }

            shimakaze::scheduler::run_on_main_thread(shimakaze::loading::replace_to_menu_layer);
            return;
        }

        // we're creating an arbitrary progress limit for the progress bar
        float arbitrary_limit = 5.0f;

        shimakaze::loading::update_progress_text("Setup: Loading Resources...");
        shimakaze::loading::update_progress(arbitrary_limit, 1.0f);

        // get texture & sprite frame cache
        auto texture_cache = CCTextureCache::sharedTextureCache();
        auto sprite_frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();

        // load textures
        texture_cache->addImage("shimakaze-GameSheet01.png", false);
        sprite_frame_cache->addSpriteFramesWithFile("shimakaze-GameSheet01.plist");

        shimakaze::loading::update_progress_text("Setup: Starting v8 Engine...");

        // create a platform for v8
        std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform.get());

        // initialize v8
        v8::V8::Initialize();

        shimakaze::loading::update_progress_text("Setup: Creating v8 Environment...");
        shimakaze::loading::update_progress(arbitrary_limit, 2.0f);

        // create a v8 isolate environment
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        v8::Isolate *isolate = v8::Isolate::New(create_params);

        // set isolate globally
        g_isolate = isolate;

        isolate->SetHostImportModuleDynamicallyCallback(module::dynamic_call);
        isolate->SetHostInitializeImportMetaObjectCallback(module::import_meta);

        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);

        shimakaze::loading::update_progress_text("Setup: Creating v8 Context...");
        shimakaze::loading::update_progress(arbitrary_limit, 3.0f);

        // create a main context for the isolate
        v8::Local<v8::Context> main_context = create_main_context(isolate);
        g_global_context.Reset(isolate, main_context);

        // retain the livelihood of the pointer and release it
        g_platform = platform.release();

        // table data
        toml::table *shimakaze_main = g_config["main"].as_table();
        bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

        console::debug_if("Shimakaze", "Successfully loaded configuration file", show_debug);

        /// STANDARD LIBRARY SECTION STARTies
        v8::Local<v8::Module> standard = standard::create_standard_module(isolate);
        v8::Local<v8::Module> cocos = standard::create_cocos_module(isolate);
        v8::Local<v8::Module> gd = standard::create_gd_module(isolate);

        // add the libraries
        handler::add_library(isolate, "shimakaze", standard);
        handler::add_library(isolate, "cocos2d", cocos);
        handler::add_library(isolate, "gd", gd);

        /// STANDARD LIBRARY SECTION END
        shimakaze::loading::update_progress_text("Setup: Precalculating amount of mods...");
        shimakaze::loading::update_progress(arbitrary_limit, 4.0f);

        // set temp image as search path for cocos
        CCFileUtils *fileUtils = CCFileUtils::sharedFileUtils();
        std::vector<std::string> searchPaths = fileUtils->getSearchPaths();
        searchPaths.insert(searchPaths.begin(), shimakaze_temp_images_path.string());
        fileUtils->setSearchPaths(searchPaths);

        /// MODLOADER SECTION START
        std::vector<std::filesystem::path> starting_mod_files;
        console::debug_if("Shimakaze", "Adding first set of mods to vector", show_debug);

        // set base mod count to 0
        g_mod_count = 0;

        // read mod folder directory
        for (const auto &entry : std::filesystem::directory_iterator(shimakaze_mods_path))
        {
            // push the mods to the folder
            const std::filesystem::path entry_path = entry.path();

            // assert entry is a directory
            if (std::filesystem::is_directory(entry_path))
            {
                // push the path to the vector
                starting_mod_files.push_back(entry_path);
                g_mod_count++;
            }
        }

        // arbitrary setup loading limit complete
        shimakaze::loading::update_progress_text("Setup: Complete");
        shimakaze::loading::update_progress(arbitrary_limit, arbitrary_limit);

        // run the loop under a new thread
        g_mod_loaded_count = 0;
        g_started = true;
        handler::run_mod_set(isolate, starting_mod_files);
    }

    void dispose(v8::Isolate *isolate)
    {
        // deconstruct v8
        console::info("Shimakaze", "Unloading Shimakaze");
        isolate->Dispose();

        v8::V8::Dispose();
        v8::V8::DisposePlatform();

        // delete leftover objects
        delete g_platform;
    }

    void run_under_main_context(std::string name, v8::Local<v8::Context> context)
    {
        // get the main isolate
        v8::Isolate *main_isolate = context->GetIsolate();
        v8::Context::Scope context_scope(context);
        {
            // our scope!
            v8::HandleScope handle_scope(main_isolate);
            v8::Local<v8::String> modName = bind::to_v8(main_isolate, "(shimakaze)");

            v8::TryCatch try_catch(main_isolate);
            v8::Local<v8::String> source = bind::to_v8(main_isolate, "\"hi\"");

            v8::Local<v8::Script> script;

            bool compiled = v8::Script::Compile(context, source).ToLocal(&script);
            if (!compiled)
            {
                std::cout << *try_catch.Exception() << std::endl;
            }
            else
            {
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

    bool assert_configuration()
    {
        // get config
        toml::table config;
        std::ifstream config_file(shimakaze_config_path);

        if (!config_file.is_open())
        {
            // this is not okay.
            // throw a fatal error
            console::fatal("Shimakaze", "A fatal error has occured within the modloader.");
            console::fatal("Shimakaze", "The \"shimakaze.toml\" file is inaccessible.");

            return false;
        }

        // pass the config to a string stream
        std::stringstream buf;
        buf << config_file.rdbuf();

        // parse the config
        try
        {
            config = toml::parse(buf.str());
        }
        catch (const std::exception &e)
        {
            // this is ALSO not okay.
            console::fatal("Shimakaze", "A fatal error has occured within the modloader.");
            console::fatal("Shimakaze", "The \"shimakaze.toml\" file is invalid.");

            return false;
        }

        // set config
        g_config = config;
        return true;
    }

    void assert_shimakaze_directory()
    {
        // get default config
        toml::table default_config = SHIMAKAZE_DEFAULT_CONFIG;

        // assert the shimakaze directory exists
        if (!std::filesystem::is_directory(shimakaze_path))
        {
            std::filesystem::create_directory(shimakaze_path);
        }

        // assert the mods directory exists
        if (!std::filesystem::is_directory(shimakaze_mods_path))
        {
            std::filesystem::create_directory(shimakaze_mods_path);
        }

        // assert temp folder exists
        if (!std::filesystem::is_directory(shimakaze_temp_path))
        {
            std::filesystem::create_directory(shimakaze_temp_path);
        }
        else
        {
            // assert temp folders exist
            if (!std::filesystem::is_directory(shimakaze_temp_images_path))
            {
                std::filesystem::create_directory(shimakaze_temp_images_path);
            }
        }

        // assert the config file exists
        if (!std::filesystem::is_regular_file(shimakaze_config_path))
        {
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

    v8::Local<v8::ObjectTemplate> create_global_object(v8::Isolate *isolate)
    {
        // create a object template for the context's globals
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

        // create submoduled objects
        v8::Local<v8::ObjectTemplate> process = context::create_process_object(isolate);

        // process;
        global->Set(isolate, "process", process);

        // runtime;
        context::create_common_runtime(isolate, global);

        return global;
    }

    v8::Local<v8::Context> create_main_context(v8::Isolate *isolate)
    {
        // return the context
        return v8::Context::New(isolate, NULL, core::create_global_object(isolate));
    }
}