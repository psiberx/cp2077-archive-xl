#include "Module.hpp"
#include "Red/InkSpawner.hpp"
#include "Red/Threads.hpp"

namespace
{
constexpr auto ModuleName = "InkSpawner";
constexpr auto WaitTimeout = std::chrono::milliseconds(200);
constexpr auto WaitTick = std::chrono::milliseconds(2);
}

std::string_view App::InkSpawnerModule::GetName()
{
    return ModuleName;
}

bool App::InkSpawnerModule::Load()
{
    if (!HookBefore<Raw::InkSpawner::SpawnFromExternal>(&OnSpawnExternal))
        throw std::runtime_error("Failed to hook [InkSpawner::SpawnFromExternal].");

    return true;
}

bool App::InkSpawnerModule::Unload()
{
    Unhook<Raw::InkSpawner::SpawnFromExternal>();

    return true;
}

void App::InkSpawnerModule::OnSpawnExternal(Red::Handle<Red::ink::WidgetLibraryItemInstance>&,
                                            Red::Handle<Red::ink::Widget>&,
                                            Red::Handle<Red::ink::WidgetLibraryResource>& aLocalLibrary,
                                            Red::ResourcePath aExternalLibraryPath,
                                            Red::CName)
{
    // Check if the external library is in the list and do nothing if it is
    for (const auto& externalLibrary : aLocalLibrary->externalLibraries)
        if (externalLibrary.path == aExternalLibraryPath)
            return;

    // Add the requested library to the list
    aLocalLibrary->externalLibraries.EmplaceBack(aExternalLibraryPath);

    // Load requested library for the spawner
    auto externalLibrary = aLocalLibrary->externalLibraries.End() - 1;

    if (Raw::IsMainThread())
    {
        externalLibrary->Load();
    }
    else
    {
        externalLibrary->LoadAsync();

        const auto start = std::chrono::steady_clock::now();

        while (!externalLibrary->IsLoaded() && !externalLibrary->IsFailed())
        {
            std::this_thread::sleep_for(WaitTick);

            if (std::chrono::steady_clock::now() - start >= WaitTimeout)
                break;
        }
    }
}
