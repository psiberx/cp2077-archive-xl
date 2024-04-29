#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct EntityOverrideUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, Core::Vector<std::string>> overrides;
};
}
