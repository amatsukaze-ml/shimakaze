#pragma once

#include "handler.h"

#include "../hooks/cocos/scheduler_hook.h"

#include <optional>
#include <thread>

std::vector<std::string> loaded_modIds;
std::vector<std::string> loading_modIds;
std::vector<std::string> duplicate_modIds;

namespace shimakaze
{
    namespace core
    {
        namespace handler
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

                toml::table *modInfoTable = mod_config["mod.info"].as_table();

                if (modInfoTable)
                {
                    console::error("Shimakaze", std::format("There was a problem starting the \"{}\" mod.", *modId).c_str());
                    console::error("Shimakaze", "The mod's information table was missing.");

                    return toml::table{};
                }

                // ....yeah.
                return mod_config;
            }

            void run_mod_set(v8::Isolate *isolate, std::vector<std::filesystem::path> mods)
            {
                std::vector<std::filesystem::path> remaining_mods_files;
                toml::table *shimakaze_main = g_config["main"].as_table();
                bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

                if (scheduler::in_context_runner == true)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                }

                for (const std::filesystem::path entry_path : mods)
                {
                    console::debug_if("Shimakaze", std::format("Reading item at \"{}\"", entry_path.string()).c_str(), show_debug);

                    if (std::filesystem::is_directory(entry_path))
                    {
                        // this is a mod directory
                        // get the mod's toml configuration
                        const std::filesystem::path toml_path = entry_path / "mod.toml";

                        // parse it as a toml file
                        toml::table mod_config = handler::read_mod_config_file(toml_path);

                        std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();
                        std::optional<std::string> mod_name = mod_config["mod.info"]["display_name"].value<std::string>();

                        if (!mod_id)
                        {
                            continue;
                        }

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

                            continue;
                        }

                        // read mod dependencies
                        auto mod_dependencies = mod_config["mod"]["dependencies"];

                        if (mod_dependencies.is_array())
                        {
                            toml::array deps = mod_dependencies.ref<toml::array>();
                            bool unmet_dependencies = false;
                            bool cancel_loading = false;

                            for (auto &element : deps)
                            {
                                if (element.is_string())
                                {
                                    // this is a string, read over it
                                    std::string mod_dependency = element.ref<std::string>();
                                    std::string mod_idref = *mod_id;

                                    if (g_mod_map.count(mod_dependency.c_str()) == 0)
                                    {
                                        console::debug_if("Shimakaze", std::format("The mod {} has a dependency on the mod {} which is not loaded.", mod_idref, mod_dependency).c_str(), show_debug);

                                        unmet_dependencies = true;
                                    }

                                    if (mods.size() == 1 && unmet_dependencies == true)
                                    {
                                        // this is the last remaining mod
                                        if (g_mod_map.count(mod_dependency.c_str()) == 0)
                                        {
                                        }
                                        // it's now safe to assume that this dependency is never getting loaded
                                        console::debug_if("Shimakaze", "This is the last remaining mod, so it will not be put back into the loop. Ignoring it altogether.", show_debug);

                                        unmet_dependencies = false;
                                        cancel_loading = true;
                                        break;
                                    }

                                    console::debug_if("Shimakaze", "Pushing it back into the loop", show_debug);
                                }
                            }

                            if (unmet_dependencies)
                            {
                                // continue
                                remaining_mods_files.push_back(entry_path);
                                continue;
                            }

                            if (cancel_loading)
                            {
                                continue;
                            }
                        }

                        /// MOD.TOML READING SECTION END
                        // attach the toml table to a vector
                        g_mod_info.push_back(mod_config);

                        if (mod_name)
                        {
                            console::info("Shimakaze", std::format("Starting mod {}", *mod_name).c_str());
                        }
                        else
                        {
                            console::info("Shimakaze", std::format("Starting mod {}", *mod_id).c_str());
                        }

                        loading_modIds.push_back(*mod_id);

                        // get the mod's main file path
                        std::string mod_file = mod_config["mod"]["main_file"].ref<std::string>();
                        std::filesystem::path mod_file_path = entry_path / std::filesystem::path(mod_file);

                        // get the file as a stream
                        std::ifstream mod_file_stream(mod_file_path);
                        std::stringstream mod_stream;
                        mod_stream << mod_file_stream.rdbuf();

                        // we can now start the mod :D
                        scheduler::run_under_context(isolate, mod_stream.str(), handler::run_mod_file);
                    }
                    else if (std::filesystem::is_regular_file(entry_path))
                    {
                        // this is a .shm file
                        // TODO: load .shm files, and make a way of compiling them
                        // ignore it for now
                        continue;
                    }
                }

                if (remaining_mods_files.size() > 0)
                {
                    // we have some mods that have dependencies that are not loaded
                    // run the loop again with the remaining mods
                    run_mod_set(isolate, remaining_mods_files);
                }
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
                std::optional<std::string> mod_name = mod_config["mod.info"]["display_name"].value<std::string>();
                toml::table *mod_table = mod_config["mod"].as_table();
                toml::table *mod_info_table = mod_config["mod.info"].as_table();

                /// MODLOADER RUN FILE LOGIC START
                // get the current isolate from the scope
                v8::Isolate *isolate = v8::Isolate::GetCurrent();

                // create a context scope
                v8::Context::Scope context_scope(context);
                {
                    // create a handle scope for the current area
                    v8::HandleScope handle_scope(isolate);

                    // get current global
                    v8::TryCatch try_catch(isolate);

                    // create a script name and origin
                    v8::Local<v8::String> mod_id = bind::to_v8(isolate, mod_table->get("mod_id")->ref<std::string>());
                    v8::ScriptOrigin origin(isolate, mod_id);
                    v8::Local<v8::Script> mod_script;

                    // turn the script into a v8 local
                    v8::Local<v8::String> script = bind::to_v8(isolate, mod_file);

                    console::debug_if("Shimakaze", "Compiling mod file");
                    // compile the script
                    if (!v8::Script::Compile(context, script, &origin).ToLocal(&mod_script))
                    {
                        // oh no, possible exception?
                        log_exception(isolate, &try_catch);
                        return;
                    }

                    console::debug_if("Shimakaze", "Running mod file");
                    // run the script and save the result
                    v8::Local<v8::Value> result;
                    if (!mod_script->Run(context).ToLocal(&result))
                    {
                        // oh no, another possible exception?
                        log_exception(isolate, &try_catch);
                        return;
                    }

                    if (try_catch.HasCaught())
                    {
                        // oh no, yet another possible exception?
                        log_exception(isolate, &try_catch);
                        return;
                    }

                    if (!result->IsUndefined())
                    {
                        // oh sick, the mod file returned a value
                        // we'll throw that into the mod map
                        add_mod(isolate, mod_config, result);
                    }

                    if (mod_name)
                    {
                        console::info("Shimakaze", std::format("Finished loading and running {}", *mod_name).c_str());
                    }
                    else
                    {
                        console::info("Shimakaze", std::format("Finished loading and running {}", mod_table->get("mod_id")->ref<std::string>()).c_str());
                    }
                }

                /// MODLOADER RUN FILE LOGIC END
            }

            void add_mod(v8::Isolate *isolate, toml::table mod_config, v8::Local<v8::Value> result)
            {
                // add the reuslt to the mod map
                std::optional<std::string> mod_id = mod_config["mod"]["mod_id"].value<std::string>();

                g_mod_map.insert(
                    std::make_pair<const char *, std::tuple<toml::table, COPYABLE_PERSISTENT<v8::Value>>>(
                        (*mod_id).c_str(),
                        std::make_tuple(
                            mod_config,
                            COPYABLE_PERSISTENT<v8::Value>(isolate, result))));
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

                if (try_catch->StackTrace(context).ToLocal(&stack_trace_str) && stack_trace_str->IsString() && stack_trace_str.As<v8::String>()->Length() > 0)
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
                else
                {
                    console::error("Shimakaze", "");
                    console::error("Shimakaze", "No stack trace has been provided.");
                }
            }
        }
    }
}