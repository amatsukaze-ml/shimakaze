#include "pch.h"

#include "core/core.h"
#include "hooks/cocos/scheduler_hook.h"

DWORD WINAPI main_thread(void *hModule)
{
    // initialize minhook
    MH_Initialize();

    // create our main core hooks
    SHIMAKAZE_COCOS_HOOK("?update@CCScheduler@cocos2d@@UAEXM@Z", CCScheduler_update);

    // start the shimakaze core
    shimakaze::core::start();

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
    }

    if (ul_reason == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0x100, main_thread, hModule, 0, 0);
    }

    return TRUE;
}