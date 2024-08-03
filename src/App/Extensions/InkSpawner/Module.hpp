#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/InkSpawner.hpp"

namespace App
{
class InkSpawnerModule : public Module
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static uintptr_t OnSpawnLocal(Red::ink::WidgetLibraryResource& aLibrary,
                                  Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance,
                                  Red::CName aItemName);

    static uintptr_t OnSpawnExternal(Red::ink::WidgetLibraryResource& aLibrary,
                                     Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance,
                                     Red::ResourcePath aExternalPath,
                                     Red::CName aItemName);

    static bool OnAsyncSpawnLocal(Red::ink::WidgetLibraryResource& aLibrary,
                                  Red::InkSpawningInfo& aSpawningInfo,
                                  Red::CName aItemName);

    static bool OnAsyncSpawnExternal(Red::ink::WidgetLibraryResource& aLibrary,
                                     Red::InkSpawningInfo& aSpawningInfo,
                                     Red::ResourcePath aExternalPath,
                                     Red::CName aItemName);

    static void OnFinishAsyncSpawn(Red::InkSpawningContext& aContext,
                                   Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance);

    inline static void InjectDependency(Red::ink::WidgetLibraryResource& aLibrary,
                                        Red::ResourcePath aExternalPath);

    inline static void InjectController(Red::Handle<Red::ink::WidgetLibraryItemInstance>& aInstance,
                                        Red::CName aControllerName);

    inline static void InheritProperties(Red::IScriptable* aTarget, Red::IScriptable* aSource);

    inline static Red::SharedSpinLock s_mutex;
};
}
