#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Garment/Tags.hpp"

namespace App
{
struct GarmentOverrideConfig : ModuleConfig
{
    using ModuleConfig::ModuleConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<std::string, OverrideTagDefinition> tags;
};
}
