#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "InkSpawner";
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

void App::InkSpawnerModule::OnSpawnExternal(RED4ext::Handle<RED4ext::ink::WidgetLibraryItemInstance>&,
                                            RED4ext::Handle<RED4ext::ink::Widget>&,
                                            RED4ext::Handle<RED4ext::ink::WidgetLibraryResource>& aLocalLibrary,
                                            RED4ext::ResourcePath aExternalLibraryPath,
                                            RED4ext::CName)
{
    // Check if the external library is in the list and do nothing if it is
    for (const auto& externalLibrary : aLocalLibrary->externalLibraries)
        if (externalLibrary.path == aExternalLibraryPath)
            return;

    // Add the requested library to the list.
    aLocalLibrary->externalLibraries.EmplaceBack(aExternalLibraryPath);

    // Load requested library for the spawner.
    (aLocalLibrary->externalLibraries.End() - 1)->Load();
}
