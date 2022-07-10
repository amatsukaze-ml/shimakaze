#pragma once

#include "../pch.h"

#include "core.h"

#include "mods/mod_entry.h"

#include <map>
#include <vector>
#include <tuple>
#include <filesystem>

namespace shimakaze::core::handler
{
    inline std::map<std::string, ModEntry*> g_mod_map;
    inline std::map<std::string, COPYABLE_PERSISTENT<v8::Module>> g_libraries;
    inline std::map<std::string, toml::table> g_mod_config_map;

    toml::table read_mod_config_file(std::filesystem::path path);

    void run_mod_file(std::string mod_file, v8::Local<v8::Context> context);
    void run_mod_set(v8::Isolate* isolate, std::vector<std::filesystem::path> mods);
    void load_resource(std::filesystem::path, std::tuple<std::filesystem::path, std::filesystem::path> resource, bool imageOnly = false);
    void log_exception(v8::Isolate *isolate, v8::TryCatch *try_catch);

    void add_mod(v8::Isolate* isolate, toml::table mod_config, v8::Local<v8::Module> result);
    void add_library(v8::Isolate *isolate, std::string name, v8::Local<v8::Module> library);

    std::vector<COPYABLE_PERSISTENT<v8::Function>> get_hook_map(std::map<std::string, std::vector<COPYABLE_PERSISTENT<v8::Function>>> &hook_map, std::string name);
    void add_hook(std::map<std::string, std::vector<COPYABLE_PERSISTENT<v8::Function>>> &hook_map, std::string name, COPYABLE_PERSISTENT<v8::Function> hook);
}