#pragma once

#pragma warning(push)
#pragma warning(disable : 4251)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <cocos2d.h>
#include <MinHook.h>
#include <gd.h>
#include <toml.hpp>

/// UTILITIES 

#include "util/console.h"
#include "core/v8/convert.h"

USING_NS_CC;

namespace gd
{
    inline auto cocos_base = GetModuleHandleA("libcocos2d.dll");
}

#pragma warning(pop)

/// UTILITY TYPEDEFS

template <typename Value>
using COPYABLE_PERSISTENT = v8::Persistent<Value, v8::CopyablePersistentTraits<Value>>;

template <typename Value>
using NON_COPYABLE_PERSISTENT = v8::Persistent<Value, v8::NonCopyablePersistentTraits<Value>>;


/// UTILITY FUNCTIONS

inline std::string replace_all(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

/// CONFIG SECTION

#define SHIMAKAZE_VERSION "0.1.0-RC.1"

#define SHIMAKAZE_DEFAULT_CONFIG                       \
    toml::table                                        \
    {                                                  \
        {                                              \
            "main", toml::table                        \
            {                                          \
                {"auto_update", false},                \
                    {"show_debug", true},              \
                {                                      \
                    "use_experimental_features", false \
                }                                      \
            }                                          \
        }                                              \
    }

/// HOOKS SECTION

#define SHIMAKAZE_HOOK(name, type, ret)      \
    inline ret(__thiscall *name)(type self); \
    ret __fastcall name##_H(type self, void *)

#define SHIMAKAZE_HOOK_ARGS(name, type, ret, ...)         \
    inline ret(__thiscall *name)(type self, __VA_ARGS__); \
    ret __fastcall name##_H(type self, void *, __VA_ARGS__)

#define SHIMAKAZE_CALL(name, type, ret) \
    ret __fastcall name##_H(type self, void *)

#define SHIMAKAZE_CALL_ARGS(name, type, ret, ...) \
    ret __fastcall name##_H(type self, void *, __VA_ARGS__)

#define SHIMAKAZE_GD_HOOK(address, name)                         \
    MH_CreateHook(                                               \
        reinterpret_cast<LPVOID>(gd::base + address),            \
        reinterpret_cast<LPVOID>(&name##_H),                     \
        reinterpret_cast<LPVOID *>(&name));                      \
    MH_EnableHook(reinterpret_cast<LPVOID>(gd::base + address)); \
    shimakaze::console::info("Shimakaze", std::format("Added a Geometry Dash hook on {} created by Shimakaze", replace_all(std::string(#name), std::string("_"), std::string("::"))).c_str())

#define SHIMAKAZE_COCOS_HOOK(symbol, name)                                           \
    MH_CreateHook(                                                                   \
        reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol)),            \
        reinterpret_cast<LPVOID>(&name##_H),                                         \
        reinterpret_cast<LPVOID *>(&name));                                          \
    MH_EnableHook(reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol))); \
    shimakaze::console::info("Shimakaze", std::format("Added a cocos2d hook on {} created by Shimakaze", replace_all(std::string(#name), std::string("_"), std::string("::"))).c_str())

#define SHIMAKAZE_GD_HOOK_NOLOG(address, name)              \
    MH_CreateHook(                                               \
        reinterpret_cast<LPVOID>(gd::base + address),            \
        reinterpret_cast<LPVOID>(&name##_H),                     \
        reinterpret_cast<LPVOID *>(&name));                      \
    MH_EnableHook(reinterpret_cast<LPVOID>(gd::base + address));
#define SHIMAKAZE_COCOS_HOOK_NOLOG(symbol, name)                               \
    MH_CreateHook(                                                                   \
        reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol)),            \
        reinterpret_cast<LPVOID>(&name##_H),                                         \
        reinterpret_cast<LPVOID *>(&name));                                          \
    MH_EnableHook(reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol))); \