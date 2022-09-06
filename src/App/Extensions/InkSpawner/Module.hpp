#pragma once

#include "Raws.hpp"
#include "App/Module/ModuleBase.hpp"

namespace App
{
class InkSpawnerModule : public Module
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    static void OnSpawnExternal(RED4ext::Handle<RED4ext::ink::WidgetLibraryItemInstance>&,
                                RED4ext::Handle<RED4ext::ink::Widget>&,
                                RED4ext::Handle<RED4ext::ink::WidgetLibraryResource>& aLocalLibrary,
                                RED4ext::ResourcePath aExternalLibraryPath,
                                RED4ext::CName);
};
}
