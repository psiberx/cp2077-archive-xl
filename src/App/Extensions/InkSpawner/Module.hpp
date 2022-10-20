#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
class InkSpawnerModule : public Module
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static void OnSpawnExternal(Red::Handle<Red::ink::WidgetLibraryItemInstance>&,
                                Red::Handle<Red::ink::Widget>&,
                                Red::Handle<Red::ink::WidgetLibraryResource>& aLocalLibrary,
                                Red::ResourcePath aExternalLibraryPath,
                                Red::CName);
};
}
