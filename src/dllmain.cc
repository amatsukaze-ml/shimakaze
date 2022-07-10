#include "pch.h"

#include "core/core.h"

#include "hooks/loading_layer.h"
#include "hooks/menu_layer.h"
#include "hooks/cocos/scheduler_hook.h"

DWORD WINAPI main_thread(void *hModule)
{
    shimakaze::console::info("Shimakaze", std::format("Starting Shimakaze v{}", SHIMAKAZE_VERSION).c_str());
    shimakaze::console::info("Shimawkaze", "Running for Geometry Dash 2.1");
    shimakaze::console::info("Shimakaze", std::format("Engine: v8 {}", v8::V8::GetVersion()).c_str());

    // initialize minhook
    MH_Initialize();
    shimakaze::console::info("Shimakaze", "Initializing main hooks...");

    // create our main core hooks
    SHIMAKAZE_GD_HOOK(0x18C080, LoadingLayer_init);
    SHIMAKAZE_GD_HOOK(0x18C790, LoadingLayer_loadingFinished);
    SHIMAKAZE_GD_HOOK(0x1907B0, MenuLayer_init);
    SHIMAKAZE_COCOS_HOOK("?update@CCScheduler@cocos2d@@UAEXM@Z", CCScheduler_update);

    // assert the shimakaze files exist
    shimakaze::core::assert_shimakaze_directory();

    // assert configuration
    bool res = shimakaze::core::assert_configuration();

    if (!res)
    {
        return -1;
    }

    // show_debug
    toml::table *shimakaze_main = shimakaze::core::g_config["main"].as_table();
    bool show_debug = shimakaze_main->get("show_debug")->value_or(true);

    shimakaze::console::debug_if("Shimakaze", "Allowing Geometry Dash to finish loading...");

    return 0;
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason,
    LPVOID lpReserved)
{
    // allocate a console
    if (AllocConsole())
    {
        FILE *fDummy;
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONOUT$", "w", stdout);

        SetConsoleTitle("shimakaze");
    }

    if (ul_reason == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0x1000, main_thread, hModule, 0, 0);
    }

    return TRUE;
}