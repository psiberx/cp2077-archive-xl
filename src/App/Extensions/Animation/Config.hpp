#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct AnimationEntry
{
    AnimationEntry(std::string aEntity, std::string aAnimSet)
        : entity(std::move(aEntity))
        , set(std::move(aAnimSet))
    {
    }

    std::string entity;
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
