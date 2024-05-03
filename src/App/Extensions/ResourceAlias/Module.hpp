#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/ResourceAlias/Unit.hpp"

namespace App
{
class ResourceAliasModule : public ConfigurableUnitModule<ResourceAliasUnit>
{
public:
    static constexpr auto CustomizationAlias = Red::ResourcePath("player_customization.app");

    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

    static const Core::Set<Red::ResourcePath>& ResolveAlias(Red::ResourcePath aAliasPath);
    static bool IsAliased(Red::ResourcePath aAliasPath, Red::ResourcePath aTargetPath);

private:
    void InitializeAliases();

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_aliases;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
