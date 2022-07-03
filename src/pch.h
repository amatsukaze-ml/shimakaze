#pragma once

#pragma warning( push )
#pragma warning( disable : 4251 )

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <cocos2d.h>
#include <MinHook.h>
#include <gd.h>

USING_NS_CC;

namespace gd {
    inline auto cocos_base = GetModuleHandleA("libcocos2d.dll");
}

#pragma warning( pop )


#define SHIMAKAZE_HOOK(name, type, ret) \
    inline ret (__thiscall* name)(type self); \
    ret __fastcall name##_H(type self, void*)

#define SHIMAKAZE_HOOK_ARGS(name, type, ret, ...) \
    inline ret (__thiscall* name)(type self, __VA_ARGS__); \
    ret __fastcall name##_H(type self, void*, __VA_ARGS__)

#define SHIMAKAZE_CALL(name, type, ret) \
    ret __fastcall name##_H(type self, void*)

#define SHIMAKAZE_CALL_ARGS(name, type, ret, ...) \
    ret __fastcall name##_H(type self, void*, __VA_ARGS__)

#define SHIMAKAZE_GD_HOOK(address, name) \
    MH_CreateHook( \
        reinterpret_cast<LPVOID>(gd::base + address), \
        reinterpret_cast<LPVOID>(&name##_H), \
        reinterpret_cast<LPVOID*>(&name) \
    ); \
    MH_EnableHook(reinterpret_cast<LPVOID>(gd::base + address))
    
#define SHIMAKAZE_COCOS_HOOK(symbol, name) \
    MH_CreateHook( \
        reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol)), \
        reinterpret_cast<LPVOID>(&name##_H), \
        reinterpret_cast<LPVOID*>(&name) \
    ); \
    MH_EnableHook(reinterpret_cast<LPVOID>(GetProcAddress(gd::cocos_base, symbol)))