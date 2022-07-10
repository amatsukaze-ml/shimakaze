#pragma once

#pragma warning(push)
#pragma warning(disable : 4251)

// Shut the fuck up lmao
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

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
#include "bind/convert.h"

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

template<typename T1, typename T2>
T1& from_offset(T2 object, unsigned int offset) {
    return *reinterpret_cast<T1*>(reinterpret_cast<uintptr_t>(object) + offset);
}

template <class T = cocos2d::CCNode *>
static T getChild(cocos2d::CCNode *x, int i)
{
    // start from end for negative index
    if (i < 0)
        i = x->getChildrenCount() + i;
    // check if backwards index is out of bounds
    if (i < 0)
        return nullptr;
    // check if forwards index is out of bounds
    if (static_cast<int>(x->getChildrenCount()) <= i)
        return nullptr;
    return reinterpret_cast<T>(x->getChildren()->objectAtIndex(i));
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

/// lifesaver section

#define DEFINE_FIELD(class_name, field, type, converter) \
    void Get##class_name##field(Local<String> name, const PropertyCallbackInfo<Value>& info) { \
        void* value = _get##class_name<Value>(name, info)->field##_; \
        info.GetReturnValue().set(type::New(value)); \
    } \
    void Set##class_name##field(Local<String> name, Local<Value> value, const PropertyCallbackInfo<Value>& info) { \
        class_name* pt = _get##class_name<void>(name, info) \
        pt->field##_ = value->converter(); \
    }

/// HOOKS SECTION

#define SHIMAKAZE_HOOK(name, type, ret)      \
    inline ret(__thiscall *name)(type self); \
    ret __fastcall name##_H(type self, void *)

#define SHIMAKAZE_HOOK_ARGS(name, type, ret, ...)         \
    inline ret(__thiscall *name)(type self, __VA_ARGS__); \
    ret __fastcall name##_H(type self, void *, __VA_ARGS__)

#define SHIMAKAZE_STATIC_HOOK(name, type, ret) \
    inline ret(__fastcall name)(type self);    \
    ret __fastcall name##_H(type self, void *)

#define SHIMAKAZE_STATIC_HOOK_ARGS(name, type, ret, ...) \
    inline ret(__thiscall name)(type self, __VA_ARGS__); \
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

#define SHIMAKAZE_GD_HOOK_NOLOG(address, name)        \
    MH_CreateHook(                                    \
        reinterpret_cast<LPVOID>(gd::base + address), \
        reinterpret_cast<LPVOID>(&name##_H),          \
        reinterpret_cast<LPVOID *>(&name));           \
    MH_EnableHook(reinterpret_cast<LPVOID>(gd::base + address));
#define SHIMAKAZE_COCOS_HOOK_NOLOG(symbol, name)                                     \
    MH_CreateHook(                                                                   \
        reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol)),            \
        reinterpret_cast<LPVOID>(&name##_H),                                         \
        reinterpret_cast<LPVOID *>(&name));                                          \
    MH_EnableHook(reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol))); \
    \