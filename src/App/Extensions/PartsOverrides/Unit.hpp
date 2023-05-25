#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PartsOverrides/Tags.hpp"

namespace App
{
struct PartsOverridesUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<std::string, OverrideTagDefinition> tags;
};
}
