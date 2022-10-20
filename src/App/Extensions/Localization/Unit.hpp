#pragma once

#include "App/Extensions/ModuleBase.hpp"

namespace App
{
struct LocalizationUnit : ConfigurableUnit
{
    using ConfigurableUnit::ConfigurableUnit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Red::CName fallback;
    Core::Map<Red::CName, Core::Vector<std::string>> onscreens;
    Core::Map<Red::CName, Core::Vector<std::string>> subtitles;
};
}
