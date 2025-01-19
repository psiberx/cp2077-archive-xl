#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct AnimationEntry
{
    Core::Set<std::string> entities;
    std::string component = "root";
    std::string set;
    uint8_t priority = 128;
    Core::Vector<std::string> variables;
};

struct AnimationsConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<AnimationEntry> animations;
};
}
