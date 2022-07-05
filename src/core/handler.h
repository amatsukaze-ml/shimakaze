#pragma once

#include "../pch.h"

#include "core.h"

#include <map>
#include <vector>
#include <tuple>
#include <filesystem>

namespace shimakaze
{
    namespace core
    {
        namespace handler
        {
            inline std::map<const char *, std::tuple<toml::table, COPYABLE_PERSISTENT<v8::Value>>> g_mod_map;

            toml::table read_mod_config_file(std::filesystem::path path);

            void run_mod_file(std::string mod_file, v8::Local<v8::Context> context);
            void run_mod_set(v8::Isolate* isolate, std::vector<std::filesystem::path> mods);
            void log_exception(v8::Isolate *isolate, v8::TryCatch *try_catch);

            void add_mod(v8::Isolate* isolate, toml::table mod_config, v8::Local<v8::Value> result);
        }
    }
}