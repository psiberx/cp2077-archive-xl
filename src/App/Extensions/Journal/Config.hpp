#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct JournalConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> journals;
};
}
