#include "App/Application.hpp"
#include "Core/Facades/Hook.hpp"
#include "Core/Facades/Runtime.hpp"
#include "Core/Runtime/OwnerMutex.hpp"
#include "Engine/Raws.hpp"
#include "Project.hpp"

namespace
{
Core::UniquePtr<App::Application> g_app;
Core::UniquePtr<Core::OwnerMutex> g_mutex;
}

// RED4ext

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason,
                                        const RED4ext::Sdk* aSdk)
{
    switch (aReason)
    {
    case RED4ext::EMainReason::Load:
    {
        g_mutex = Core::MakeUnique<Core::OwnerMutex>(Project::Name);

        if (g_mutex->Obtain())
        {
            g_app = Core::MakeUnique<App::Application>(aHandle, aSdk);
            g_app->Bootstrap();
        }
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        if (g_mutex->IsOwner())
        {
            g_app->Shutdown();
            g_app = nullptr;

            g_mutex->Release();
            g_mutex = nullptr;
        }
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = Project::Name;
    aInfo->author = Project::Author;
    aInfo->version = RED4EXT_SEMVER(Project::Version.major, Project::Version.minor, Project::Version.patch);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}

// ASI

BOOL APIENTRY DllMain(HMODULE aHandle, DWORD aReason, LPVOID)
{
    static const bool s_isASI = Core::Runtime::IsASI(aHandle) && Core::Runtime::IsEXE(L"Cyberpunk2077.exe");

    switch (aReason) // NOLINT(hicpp-multiway-paths-covered)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(aHandle);

        if (s_isASI)
        {
            g_mutex = Core::MakeUnique<Core::OwnerMutex>(Project::Name);

            if (g_mutex->Obtain())
            {
                g_app = Core::MakeUnique<App::Application>(aHandle);

                Core::Hook::Before<Engine::Raw::Main>(+[] {
                    g_app->Bootstrap();
                });
            }
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        if (s_isASI && g_mutex->IsOwner())
        {
            g_app->Shutdown();
            g_app = nullptr;

            g_mutex->Release();
            g_mutex = nullptr;
        }
        break;
    }
    }

    return TRUE;
}
