#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/ResourceMeta/Config.hpp"

namespace App
{
class ResourceMetaExtension : public ConfigurableExtensionImpl<ResourceMetaConfig>
{
public:
    static constexpr auto CustomizationApp = Red::ResourcePath("player_customization.app");

    std::string_view GetName() override;
    void Configure() override;

    static bool InScope(Red::ResourcePath aScopePath, Red::ResourcePath aTargetPath);
    static const Core::Set<Red::ResourcePath>& GetList(Red::ResourcePath aScopePath);

    static Core::Set<std::string> ExpandList(const Core::Set<std::string>& aList);
    static Core::Set<Red::ResourcePath> ExpandList(const Core::Set<Red::ResourcePath>& aList);
    static Core::Map<Red::ResourcePath, std::string> ExpandList(const Core::Map<Red::ResourcePath, std::string>& aList);

    static const ResourceFix& GetFix(Red::ResourcePath aTargetPath);

private:
    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_scopes;
    inline static Core::Map<Red::ResourcePath, ResourceFix> s_fixes;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
