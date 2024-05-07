#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct ResourcePatchScope
{
    Core::Set<Red::ResourcePath> includes;
    Core::Set<Red::ResourcePath> excludes;
};

struct ResourcePatchUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, ResourcePatchScope> patches;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
