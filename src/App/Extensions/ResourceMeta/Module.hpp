#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/ResourceMeta/Unit.hpp"

namespace App
{
class ResourceMetaModule : public ConfigurableUnitModule<ResourceMetaUnit>
{
public:
    static constexpr auto CustomizationApp = Red::ResourcePath("player_customization.app");

    std::string_view GetName() override;
    void Configure() override;

    static const Core::Set<Red::ResourcePath>& GetResourceList(Red::ResourcePath aAliasPath);
    static bool IsInResourceList(Red::ResourcePath aAliasPath, Red::ResourcePath aTargetPath);

    static const ResourceFix& GetResourceFix(Red::ResourcePath aTargetPath);

private:
    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_aliases;
    inline static Core::Map<Red::ResourcePath, ResourceFix> s_fixes;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
