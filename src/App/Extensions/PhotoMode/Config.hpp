#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct PhotoModeConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
};
}
