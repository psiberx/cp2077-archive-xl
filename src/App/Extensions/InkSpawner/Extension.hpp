#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "Red/InkLibrary.hpp"
#include "Red/InkSpawner.hpp"

namespace App
{
class InkSpawnerExtension : public Extension
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static uintptr_t OnSpawnLocal(Red::inkWidgetLibraryResource* aLibrary,
                                  Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance,
                                  Red::CName aItemName);
    static uintptr_t OnSpawnExternal(Red::inkWidgetLibraryResource* aLibrary,
                                     Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance,
                                     Red::ResourcePath aExternalPath,
                                     Red::CName aItemName);
    static bool OnAsyncSpawnLocal(Red::inkWidgetLibraryResource* aLibrary,
                                  Red::InkSpawningInfo& aSpawningInfo,
                                  Red::CName aItemName,
                                  bool aFlag);
    static bool OnAsyncSpawnExternal(Red::inkWidgetLibraryResource* aLibrary,
                                     Red::InkSpawningInfo& aSpawningInfo,
                                     Red::ResourcePath aExternalPath,
                                     Red::CName aItemName,
                                     bool aFlag);
    static void OnFinishAsyncSpawn(Red::InkSpawningContext& aContext,
                                   Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance);

    inline static Red::Handle<Red::inkWidgetLibraryResource> ResolveDependency(Red::inkWidgetLibraryResource* aLibrary,
                                                                               Red::ResourcePath aExternalPath);
    inline static void InjectController(Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance,
                                        Red::CName aControllerName);
    inline static void InheritProperties(Red::IScriptable* aTarget, Red::IScriptable* aSource);

    inline static Red::SharedSpinLock s_mutex;
};
}
