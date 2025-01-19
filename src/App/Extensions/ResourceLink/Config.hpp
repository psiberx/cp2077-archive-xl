#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourceLinkConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> copies;
    Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> links;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
