#pragma once

#include "App/Module/ModuleBase.hpp"

namespace App
{
struct LocalizationUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    RED4ext::CName fallback;
    Core::Map<RED4ext::CName, Core::Vector<std::string>> onscreens;
    Core::Map<RED4ext::CName, Core::Vector<std::string>> subtitles;
};
}
