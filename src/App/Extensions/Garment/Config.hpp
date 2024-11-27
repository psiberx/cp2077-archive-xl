#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/Garment/Tags.hpp"

namespace App
{
struct GarmentOverrideConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<std::string, OverrideTagDefinition> tags;
};
}
