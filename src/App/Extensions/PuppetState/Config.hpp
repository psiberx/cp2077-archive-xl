#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct PuppetStateConfig : ModuleConfig
{
    using ModuleConfig::ModuleConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> bodyTypes;
};
}
