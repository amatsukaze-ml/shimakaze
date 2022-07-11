#pragma once

#include "handler.h"
#include "core.h"
#include "module.h"

#include "../hooks/menu_layer.h"
#include "../hooks/loading_layer.h"
#include "../hooks/cocos/scheduler_hook.h"

#include <optional>
#include <thread>

std::vector<std::string> loaded_modIds;
std::vector<std::string> loading_modIds;
std::vector<std::string> duplicate_modIds;

// folders
const auto base_path = std::filesystem::current_path();
const auto shimakaze_path = base_path / "shimakaze";
const auto shimakaze_mods_path = shimakaze_path / "mods";
const auto shimakaze_config_path = shimakaze_path / "shimakaze.toml";

// temp folders
const auto shimakaze_temp_path = shimakaze_path / ".temp";
const auto shimakaze_temp_images_path = shimakaze_temp_path / "images";

namespace shimakaze::core::handler
{
    toml::table read_mod_config_file(std::filesystem::path path)
    {
        std::filesystem::path entry_path = path.string().substr(0, path.string().length() - 8);
        std::ifstream toml_filestream(path);

        if (!toml_filestream.is_open())
        {
            console::error("Shimakaze", std::format("The directory at {} is an invalid mod directory", entry_path.string()).c_str());

            return toml::table{};
        }

        // read the file into a string buffer
        std::stringstream buf;
        buf << toml_filestream.rdbuf();

        toml::table mod_config;
        try
        {
            mod_config = toml::parse(buf.str());
        }
        catch (const toml::parse_error &err)
        {
            console::error("Shimakaze", std::format("There was a problem parsing the mod.toml file at the {} directory.", entry_path.string()).c_str());
            console::error("Shimakaze", std::string(err.description()).c_str());

            return toml::table{};
        }

        /// MOD.TOML READING SECTION START
        // get the [mod] table
        toml::table *modTable = mod_config["mod"].as_table();

        if (!modTable)
        {
            console::error("Shimakaze", std::format("There was a problem parsing the mod.toml file at the {} directory.", entry_path.string()).c_str());
            console::error("Shimakaze", "The [mod] table was missing.");

            return toml::table{};
        }

        // get mod file and id
        std::optional<std::string> modId = mod_config["mod"]["mod_id"].value<std::string>();
        std::optional<std::string> modFile = mod_config["mod"]["main_file"].value<std::string>();

        if (!modId || !modFile)
        {
            console::error("Shimakaze", std::format("There was a problem parsing the mod.toml file at the {} directory.", entry_path.string()).c_str());
            console::error("Shimakaze", "The mod_id or main_file key is missing a value.");

            return toml::table{};
        }

        // validate the mod file even exists
        std::string modFilePath = entry_path.string() + "/" + *modFile;
        std::ifstream imodfile(modFilePath.c_str());

        if (!imodfile.is_open())
        {
            console::error("Shimakaze", std::format("There was a problem starting the \"{}\" mod.", *modId).c_str());
            console::error("Shimakaze", "The assigned startup mod file was missing.");

            return toml::table{};
        }

        toml::table *modInfoTable = mod_config["mod"]["info"].as_table();

        if (!modInfoTable)
        {
            console::error("Shimakaze", std::format("There was a problem starting the \"{}\" mod.", *modId).c_str());
            console::error("Shimakaze", "The mod's information table was missing.");

            return toml::table{};
        }

        // ....yeah.
        return mod_config;
    }

    void run_mod_set(v8::Isolate *isolate, std::vector<std::filesystem::path> mod_paths)
    {
        bool show_debug = g_config["main"]["show_debug"].ref<bool>();

        // wait until context runner is done
        while (scheduler::in_context_runner == true)
        {
            if (scheduler::in_context_runner == false)
            {
                break;
            }

            // sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // update progress bar
        shimakaze::loading::update_progress((float)shimakaze::core::g_mod_count, (float)shimakaze::core::g_mod_loaded_count);

        // mod_id, mod_path
        std::map<std::string, std::tuple<std::filesystem::path, toml::table>> valid_mod_paths;

        for (const auto mod_path : mod_paths)
        {
            console::debug_if("Shimakaze", std::format("Reading potential item at \"{}\"", mod_path.string()).c_str(), show_debug);

            if (std::filesystem::is_directory(mod_path))
            {
                // mod directory
                const auto config_path = mod_path / "mod.toml";

                // update progress text
                shimakaze::loading::update_progress_text(std::format("Initializing: Reading configuration for possible mod \"{}\"...", mod_path.filename().string()).c_str());

                // parse it as a toml file
                toml::table mod_config = handler::read_mod_config_file(config_path);

                std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();
                std::optional<std::string> mod_name = mod_config["mod"]["info"]["display_name"].value<std::string>();

                if (!mod_id)
                {
                    // decrease mod count as this one is invalid
                    shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;

                    continue;
                }

                shimakaze::loading::update_progress_text(std::format("Initializing: Reading configuration for mod \"{}\"...", *mod_id).c_str());

                // we can finally use mod id in the debug
                auto mod_count = std::ranges::count(loaded_modIds.begin(), loaded_modIds.end(), *mod_id);
                if (mod_count > 0)
                {
                    // the mod exists, and is already loaded
                    duplicate_modIds.push_back(*mod_id);

                    // get amount of times it's duplicated
                    auto size = std::ranges::count(duplicate_modIds.begin(), duplicate_modIds.end(), *mod_id);

                    // inform the user of htis
                    console::error("Shimakaze", std::format("There was a problem starting the \"{}\" mod.", *mod_id).c_str());
                    console::error("Shimakaze", "A mod with this id already exists!");

                    // decrease mod count as this one is invalid
                    shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;

                    continue;
                }

                // todo: validate mod path exists

                // valid mod path, all it really needs is a valid config, a valid id, and a valid main file
                valid_mod_paths.insert(
                    std::make_pair(
                        *mod_id,
                        std::make_tuple(
                            mod_path,
                            mod_config)));
            }
        }

        // run all the valid mods
        run_mod_dependencies(isolate, valid_mod_paths, {});
    }

    bool run_mod_dependencies(v8::Isolate *isolate, std::map<std::string, std::tuple<std::filesystem::path, toml::table>> mod_paths, std::vector<std::string> dependency_names)
    {
        bool show_debug = g_config["main"]["show_debug"].ref<bool>();

        if (dependency_names.size() > 0)
        {
            std::cout << "wew" << std::endl;
            // prioritize dependency loading
            for (const auto &id : dependency_names)
            {
                std::cout << id << std::endl;
                if (mod_paths.find(id) != mod_paths.end())
                {
                    // this dependency exists
                    const auto mod_path = std::get<0>(mod_paths[id]);
                    const auto mod_config = std::get<1>(mod_paths[id]);

                    bool result = run_mod(isolate, mod_paths, id, mod_config, mod_path);

                    if (result == false)
                    {
                        // mod failed to load
                        return false;
                    }
                }
                else
                {
                    // ok well clearly this doesn't exist
                    return false;
                }
            }

            return true;
        }

        // always ran upon first load
        // dependency vector is empty lmao
        for (const auto &[id, mod_pair] : mod_paths)
        {
            // wait until context runner is done
            while (scheduler::in_context_runner == true)
            {
                if (scheduler::in_context_runner == false)
                {
                    break;
                }

                // sleep
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            const auto mod_path = std::get<0>(mod_pair);
            const auto mod_config = std::get<1>(mod_pair);

            run_mod(isolate, mod_paths, id, mod_config, mod_path);
        }
    }

    bool run_mod(v8::Isolate *isolate, std::map<std::string, std::tuple<std::filesystem::path, toml::table>> mod_paths, std::string mod_id, toml::table mod_config, std::filesystem::path mod_path)
    {
        if (std::find(loading_modIds.begin(), loading_modIds.end(), mod_id) != loading_modIds.end())
        {
            // the mod is already checked
            return true;
        }

        mod_config.insert("__temp_mod_path", mod_path.string());

        // attempt to read mod dependencies
        auto mod_dependencies_view = mod_config["mod"]["dependencies"];

        if (mod_dependencies_view.is_array())
        {
            toml::array mod_dependencies = mod_dependencies_view.ref<toml::array>();
            // dependencies array
            std::vector<std::string> dependencies;

            // iterate over the array
            for (const auto &dependency : mod_dependencies)
            {
                if (dependency.is_string())
                {
                    // get the string
                    std::string dependency_string = dependency.ref<std::string>();
                    // check if mod is already loaded
                    auto mod_count = std::ranges::count(loaded_modIds.begin(), loaded_modIds.end(), dependency_string);

                    if (mod_count > 0)
                    {
                        // mod with this id is loaded
                        continue;
                    }

                    // add it to the vector
                    dependencies.push_back(dependency.ref<std::string>());
                }
            }

            if (dependencies.size() > 0)
            {
                // we have dependencies, so we need to wait for them to load
                bool result = run_mod_dependencies(isolate, mod_paths, dependencies);

                if (result == false)
                {
                    // something went wrong
                    shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;
                    return false;
                }
            }
        }

        // finally run the mod file
        // attach the toml table to a vector
        g_mod_info.push_back(mod_config);
        std::optional<std::string> mod_name = mod_config["mod"]["info"]["display_name"].value<std::string>();

        if (mod_name)
        {
            console::info("Shimakaze", std::format("Starting mod {}", *mod_name).c_str());
        }
        else
        {
            console::info("Shimakaze", std::format("Starting mod {}", mod_id).c_str());
        }

        run_mod_resources(mod_path, mod_config);

        loading_modIds.push_back(mod_id);

        // get the mod's main file path
        std::string mod_file = mod_config["mod"]["main_file"].ref<std::string>();
        std::filesystem::path mod_file_path = mod_path / std::filesystem::path(mod_file);

        // get the file as a stream
        std::ifstream mod_file_stream(mod_file_path);
        std::stringstream mod_stream;
        mod_stream << mod_file_stream.rdbuf();

        // we can now start the mod :D
        shimakaze::loading::update_progress_text(std::format("Initializing: Running \"{}\"...", mod_id).c_str());
        scheduler::run_under_context(isolate, mod_stream.str(), handler::run_mod_file);

        // sleep for a bit to let the scheduler catch up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        while (scheduler::in_context_runner == true)
        {
            if (scheduler::in_context_runner == false)
            {
                break;
            }

            // sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        return true;
    }

    void run_mod_file(std::string mod_file, v8::Local<v8::Context> context)
    {
        // grab mod config for our usecase
        toml::table mod_config = g_mod_info.at(0);
        toml::table *shimakaze_main = g_config["main"].as_table();
        g_mod_info.pop_front();

        // whether to show debug logs or not
        bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

        // save important mod info
        std::optional<std::string> mod_id = mod_config["mod"]["id"].value<std::string>();
        std::optional<std::string> mod_name = mod_config["mod"]["info"]["display_name"].value<std::string>();
        toml::table *mod_table = mod_config["mod"].as_table();
        toml::table *mod_info_table = mod_config["mod"]["info"].as_table();

        /// MODLOADER RUN FILE LOGIC START
        // get the current isolate from the scope
        v8::Isolate *isolate = context->GetIsolate();

        // create a context scope
        v8::Context::Scope context_scope(context);
        {
            // create a handle scope for the current area
            v8::HandleScope handle_scope(isolate);

            // get current global
            v8::TryCatch try_catch(isolate);

            // create a script name and origin
            v8::Local<v8::String> mod_id = bind::to_v8(isolate, mod_table->get("mod_id")->ref<std::string>());

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
            v8::Local<v8::String> mod_code = bind::to_v8(isolate, mod_file);

            // create module data
            v8::MaybeLocal<v8::Module> mod_module;
            v8::ScriptCompiler::Source source(mod_code, origin);

            console::debug_if("Shimakaze", "Compiling mod file");

            // compile the mod
            mod_module = v8::ScriptCompiler::CompileModule(isolate, &source);

            // now lets check if it exists or not
            v8::Local<v8::Module> mod;
            if (!mod_module.ToLocal(&mod))
            {
                // oh no, another possible exception?
                log_exception(isolate, &try_catch);

                // failed to load mod
                shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;

                if (shimakaze::core::g_mod_count == shimakaze::core::g_mod_loaded_count)
                {
                    // its impossible for it to NOT be equal now
                    shimakaze::core::g_mod_count = g_mod_map.size();

                    // we can now start the game
                    shimakaze::scheduler::run_on_main_thread(shimakaze::loading::replace_to_menu_layer);
                }
                return;
            }
            else
            {
                console::debug_if("Shimakaze", "Running mod file", show_debug);

                // woah a mod
                context->SetEmbedderData(0, mod_id);
                context->SetEmbedderData(1, bind::to_v8(isolate, mod_config["__temp_mod_path"].ref<std::string>()));

                if (mod_name)
                {
                    context->SetEmbedderData(0, bind::to_v8(isolate, *mod_name));
                }
                v8::Maybe<bool> result = mod->InstantiateModule(context, module::static_call);

                if (mod_name)
                {
                    context->SetEmbedderData(0, bind::to_v8(isolate, *mod_name));
                }

                if (result.IsNothing())
                {
                    console::debug_if("Shimakaze", "nothing", show_debug);
                    if (try_catch.HasCaught())
                    {
                        console::debug_if("Shimakaze", "inner exception", show_debug);
                        // oh no, yet another possible exception?
                        log_exception(isolate, &try_catch);

                        // failed to load mod
                        shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;

                        if (shimakaze::core::g_mod_count == shimakaze::core::g_mod_loaded_count)
                        {
                            // its impossible for it to NOT be equal now
                            shimakaze::core::g_mod_count = g_mod_map.size();

                            // we can now start the game
                            shimakaze::scheduler::run_on_main_thread(shimakaze::loading::replace_to_menu_layer);
                        }
                        return;
                    }

                    console::error("Shimakaze", std::format("An error has occured with the mod {}, but v8 has provided no useful insight as to why.", mod_table->get("mod_id")->ref<std::string>()).c_str());
                    return;
                }

                v8::MaybeLocal<v8::Value> mod_result = mod->Evaluate(context);

                if (try_catch.HasCaught())
                {
                    // oh no, yet another possible exception?
                    log_exception(isolate, &try_catch);

                    // failed to load mod
                    shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;

                    if (shimakaze::core::g_mod_count == shimakaze::core::g_mod_loaded_count)
                    {
                        // its impossible for it to NOT be equal now
                        shimakaze::core::g_mod_count = g_mod_map.size();

                        // we can now start the game
                        shimakaze::scheduler::run_on_main_thread(shimakaze::loading::replace_to_menu_layer);
                    }
                    return;
                }

                if (!mod_result.IsEmpty())
                {
                    // todo: this is wrong :)
                    add_mod(isolate, mod_config, mod);
                }

                if (mod_name)
                {
                    shimakaze::loading::update_progress_text(std::format("Initializing: Successfully ran and loaded \"{}\".", *mod_name).c_str());
                    console::info("Shimakaze", std::format("Finished loading and running {}", *mod_name).c_str());
                }
                else
                {
                    shimakaze::loading::update_progress_text(std::format("Initializing: Successfully ran and loaded \"{}\".", mod_table->get("mod_id")->ref<std::string>()).c_str());
                    console::info("Shimakaze", std::format("Finished loading and running {}", mod_table->get("mod_id")->ref<std::string>()).c_str());
                }

                if (shimakaze::core::g_mod_count == shimakaze::core::g_mod_loaded_count)
                {
                    // its impossible for it to NOT be equal now
                    shimakaze::core::g_mod_count = g_mod_map.size();

                    // we can now start the game
                    shimakaze::scheduler::run_on_main_thread(shimakaze::loading::replace_to_menu_layer);
                }
            }
        }

        /// MODLOADER RUN FILE LOGIC END
    }

    void run_mod_resources(std::filesystem::path entry_path, toml::table mod_config) {
        toml::table *shimakaze_main = g_config["main"].as_table();
        bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

        std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();
        std::optional<std::string> resource_folder = mod_config["mod"]["resource_folder"].value<std::string>();

        console::debug_if("Shimakaze", std::format("Attempting to find if mod \"{}\" has a valid resource folder...", *mod_id).c_str(), show_debug);

        if (resource_folder) {
            // convert the folder to a path
            std::filesystem::path resource_folder_path = entry_path / std::filesystem::path(*resource_folder);
            console::debug_if("Shimakaze", std::format("Resource folder found.", *mod_id).c_str(), show_debug);

            // validate the folder even exists
            if (!std::filesystem::exists(resource_folder_path))
            {
                console::debug_if("Shimakaze", "The mod provided a resource folder name, but the folder itself didn't exist.", show_debug);
                shimakaze::core::g_mod_count = shimakaze::core::g_mod_count - 1;
                return;
            }
            else
            {
                // attempt to load resources
                shimakaze::loading::update_progress_text(std::format("Initializing: Checking resources for mod \"{}\"...", *mod_id).c_str());
                console::debug_if("Shimakaze", std::format("Reading over resources folder for mod \"{}\"...", *mod_id).c_str(), show_debug);

                std::vector<std::filesystem::path> resources_plist;
                std::vector<std::filesystem::path> resources_png;
                std::vector<std::string> loaded_resources;

                bool icon_loaded = false;
                std::optional<std::string> mod_icon = mod_config["mod"]["info"]["icon"].value<std::string>();

                for (const auto &entry : std::filesystem::directory_iterator(resource_folder_path))
                {
                    auto loadedit = std::find(loaded_resources.begin(), loaded_resources.end(), entry.path().stem().string());
                    if (!entry.path().has_stem() || loadedit != loaded_resources.end())
                    {
                        // unknown
                        continue;
                    }

                    // ignore icons
                    if (!icon_loaded && mod_icon)
                    {
                        if (entry.path().filename() == std::filesystem::path(*mod_icon))
                        {
                            console::debug_if("Shimakaze", std::format("Icon for mod \"{}\" was found. Moving to temp directory...", *mod_id).c_str(), show_debug);
                            // set icon as loaded
                            icon_loaded = true;

                            // move icon to temp images folder
                            std::filesystem::path icon_path = shimakaze_temp_images_path / std::filesystem::path(std::format("{}-icon.png", *mod_id));
                            std::ifstream source(entry.path(), std::ios::binary);
                            std::ofstream dest(icon_path, std::ios::binary);

                            dest << source.rdbuf();

                            // close streams
                            source.close();
                            dest.close();

                            // get texture cache
                            auto texture_cache = CCTextureCache::sharedTextureCache();

                            // TODO: https://discord.com/channels/993695985580654742/993704576123408475/995865751082770462
                            // load icon
                            texture_cache->removeTextureForKey(icon_path.string().c_str()); // just in case.
                            texture_cache->addImage(icon_path.string().c_str(), false);

                            continue;
                        }
                    }

                    if (entry.path().extension() == ".plist")
                    {
                        // plist file
                        auto it = std::find(resources_png.begin(), resources_png.end(), entry.path().stem());
                        if (it != resources_png.end())
                        {
                            // add to loaded resources
                            loaded_resources.push_back(entry.path().stem().string());

                            // remove png from list
                            std::erase(resources_png, *it);
                        }
                        else
                        {
                            // add to resources plist
                            resources_plist.push_back(entry.path().stem());
                        }
                    }
                    else if (entry.path().extension() == ".png")
                    {
                        // png file
                        auto it = std::find(resources_plist.begin(), resources_plist.end(), entry.path().stem());
                        if (it != resources_plist.end())
                        {
                            // add to loaded_resources
                            loaded_resources.push_back(entry.path().stem().string());

                            // remove png from list
                            std::erase(resources_plist, *it);
                        }
                        else
                        {
                            // add to resources png
                            resources_png.push_back(entry.path().stem());
                        }
                    }
                }

                // don't uselessly clog the file cache vector
                bool setSearch = false;
                if (loaded_resources.size() > 0)
                {
                    // add file path to CCFileUtils
                    CCFileUtils *fileUtils = CCFileUtils::sharedFileUtils();
                    std::vector<std::string> searchPaths = fileUtils->getSearchPaths();
                    searchPaths.insert(searchPaths.begin(), resource_folder_path.string());
                    fileUtils->setSearchPaths(searchPaths);

                    // this is true now
                    setSearch = true;

                    // loading all possible resources
                    for (const auto &resource : loaded_resources)
                    {
                        shimakaze::loading::update_progress_text(std::format("Initializing: Loading resource \"{}\" for mod \"{}\"...", resource, *mod_id).c_str());

                        // load the resource
                        handler::load_resource(
                                resource_folder_path,
                                std::make_tuple<std::filesystem::path, std::filesystem::path>(
                                        resource_folder_path / std::filesystem::path(resource + ".plist"),
                                        resource_folder_path / std::filesystem::path(resource + ".png")),
                                false);
                    }
                }

                // load pngs
                if (resources_png.size() > 0)
                {
                    if (setSearch == false)
                    {
                        // add file path to CCFileUtils
                        // it didn't before
                        CCFileUtils *fileUtils = CCFileUtils::sharedFileUtils();
                        std::vector<std::string> searchPaths = fileUtils->getSearchPaths();
                        searchPaths.insert(searchPaths.begin(), resource_folder_path.string());
                        fileUtils->setSearchPaths(searchPaths);
                    }

                    // loading all possible images
                    for (const auto &resource : loaded_resources)
                    {
                        shimakaze::loading::update_progress_text(std::format("Initializing: Loading resource \"{}\" for mod \"{}\"...", resource, *mod_id).c_str());

                        // load the resource
                        handler::load_resource(
                                resource_folder_path,
                                std::make_tuple<std::filesystem::path, std::filesystem::path>(
                                        resource_folder_path / std::filesystem::path(resource + ".plist"),
                                        resource_folder_path / std::filesystem::path(resource + ".png")),
                                true);
                    }
                }
            }
        }
    }

    void load_resource(std::filesystem::path, std::tuple<std::filesystem::path, std::filesystem::path> resource, bool imageOnly)
    {
        // this is a very volatile function
        // undoubtedly, this runs under the assumption that both a .plist and a .png file are present
        // (sometimes)

        // regardless, get the shared texture cache & sprite frame cache
        auto texture_cache = CCTextureCache::sharedTextureCache();
        auto sprite_frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();

        if (imageOnly == true)
        {
            // add the png
            texture_cache->addImage(std::get<0>(resource).string().c_str(), false);

            // nothing else
            return;
        }

        // add the plist
        sprite_frame_cache->addSpriteFramesWithFile(std::get<1>(resource).string().c_str());
    }

    void add_mod(v8::Isolate *isolate, toml::table mod_config, v8::Local<v8::Module> result)
    {
        // add the reuslt to the mod map
        std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();

        g_mod_map.insert(
            std::make_pair<const char *, ModEntry *>(
                (*mod_id).c_str(),
                new ModEntry(mod_config,
                             COPYABLE_PERSISTENT<v8::Module>(isolate, result))));

        // update count
        shimakaze::core::g_mod_loaded_count = shimakaze::core::g_mod_loaded_count + 1;

        // update progress bar
        shimakaze::loading::update_progress((float)shimakaze::core::g_mod_count, (float)shimakaze::core::g_mod_loaded_count);
    }

    void add_library(v8::Isolate *isolate, std::string name, v8::Local<v8::Module> library)
    {
        // insert the module
        g_libraries.insert(
            std::make_pair<const char *, COPYABLE_PERSISTENT<v8::Module>>(
                name.c_str(),
                COPYABLE_PERSISTENT<v8::Module>(isolate, library)));
    }

    void log_exception(v8::Isolate *isolate, v8::TryCatch *try_catch)
    {
        // create a handle scope for handling the exception
        v8::HandleScope handle_scope(isolate);

        // put the message into a variable
        v8::Local<v8::Message> message = try_catch->Message();

        // get the exception string
        v8::String::Utf8Value exception(isolate, try_catch->Exception());
        const char *exception_string = *exception;

        if (message.IsEmpty())
        {
            // unfortunately, v8 has provided us with nothing useful
            // may as well try to print the exception anyways

            // stringify the resource name first
            v8::String::Utf8Value resource_name(isolate, message->GetScriptOrigin().ResourceName());
            const char *resource_name_cstr = *resource_name;

            // then log the error
            console::error("Shimakaze", std::format("An error has occured with the mod {}, but v8 has provided no useful insight as to why.", resource_name_cstr).c_str());
            console::error("Shimakaze", exception_string);
            return;
        }

        // get the rsource name as a string
        v8::String::Utf8Value resource_name(isolate, message->GetScriptOrigin().ResourceName());
        const char *resource_name_cstr = *resource_name;

        // get the current context
        v8::Local<v8::Context> context(isolate->GetCurrentContext());

        // get the line number
        int line_number = message->GetLineNumber(context).FromJust();

        // print the source line
        v8::String::Utf8Value source_line(isolate, message->GetSourceLine(context).ToLocalChecked());
        const char *source_line_cstr = *source_line;

        // create column whitespace
        std::string column_whitespace = "";
        std::string header = std::format("{}:{} {}", resource_name_cstr, line_number, source_line_cstr);
        int start_column = message->GetStartColumn(context).FromJust();

        for (int i = 0; i < start_column + header.length() - strlen(source_line_cstr); i++)
        {
            column_whitespace += " ";
        }

        // create arrow whitespace
        std::string arrow_whitespace = "";
        int end_column = message->GetEndColumn(context).FromJust();

        for (int i = start_column; i < end_column; i++)
        {
            arrow_whitespace += "^";
        }

        // print the stacktrace
        v8::Local<v8::Value> stack_trace_str;

        // print the error
        console::error("Shimakaze", std::format("An error has occured with the mod {}, at line {}.", resource_name_cstr, line_number).c_str());
        console::error("Shimakaze", header.c_str());
        console::error("Shimakaze", std::format("{}{}", column_whitespace, arrow_whitespace).c_str());

        if (try_catch->StackTrace(context).ToLocal(&stack_trace_str))
        {
            if (stack_trace_str->IsString() && stack_trace_str.As<v8::String>()->Length() > 0)
            {
                // we validated this is printable
                v8::String::Utf8Value stack_trace(isolate, stack_trace_str);
                const char *stack_trace_cstr = *stack_trace;

                // segment the stacktrace
                std::stringstream segment(stack_trace_cstr);
                std::string output;

                while (std::getline(segment, output, '\n'))
                {
                    console::error("Shimakaze", output.c_str());
                }
            }
        }
        else
        {
            console::error("Shimakaze", "");
            console::error("Shimakaze", "No stack trace has been provided.");
        }
    }

    std::vector<COPYABLE_PERSISTENT<v8::Function>> get_hook_map(std::map<std::string, std::vector<COPYABLE_PERSISTENT<v8::Function>>> &hook_map, std::string name)
    {
        // get the hook map
        auto it = hook_map.find(name);

        // if it doesn't exist, create it
        if (it == hook_map.end())
        {
            auto vect = std::vector<COPYABLE_PERSISTENT<v8::Function>>();

            hook_map.insert_or_assign(name, vect);

            return vect;
        }
        else
        {
            return (*it).second;
        }
    }

    void add_hook(std::map<std::string, std::vector<COPYABLE_PERSISTENT<v8::Function>>> &hook_map, std::string name, COPYABLE_PERSISTENT<v8::Function> hook)
    {
        // get the hook map
        auto it = hook_map.find(name);

        // if it doesn't exist, create it
        if (it == hook_map.end())
        {
            auto vect = std::vector<COPYABLE_PERSISTENT<v8::Function>>();

            vect.insert(vect.end(), hook);

            hook_map.insert(std::make_pair(name, vect));
        }
        else
        {
            auto vect = (*it).second;

            vect.insert(vect.end(), hook);

            hook_map.insert_or_assign(name, vect);
        }
    }
}