#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct ResourceLinkConfig : ModuleConfig
{
    using ModuleConfig::ModuleConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> links;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
