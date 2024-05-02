#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct ResourceAliasUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, Core::Vector<Red::ResourcePath>> aliases;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
