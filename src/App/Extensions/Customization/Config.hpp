#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct CustomizationConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> maleOptions;
    Core::Vector<std::string> femaleOptions;

private:
    bool ReadOptions(const YAML::Node& aNode, Core::Vector<std::string>& aOptions);
};
}
