#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct StreamingUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> blocks;
};
}
