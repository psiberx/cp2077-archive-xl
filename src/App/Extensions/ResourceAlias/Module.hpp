#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/ResourceAlias/Unit.hpp"

namespace App
{
class ResourceAliasModule : public ConfigurableUnitModule<ResourceAliasUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

    static const Core::Set<Red::ResourcePath>& ResolveAlias(Red::ResourcePath aAliasPath);

private:
    void InitializeAliases();

    inline static Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> s_aliases;
    inline static Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
