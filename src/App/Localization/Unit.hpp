#pragma once

#include "App/Common/Unit.hpp"

namespace App
{
struct LocalizationUnit : Unit
{
    using Unit::Unit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    RED4ext::CName fallback;
    Core::Map<RED4ext::CName, Core::Vector<std::string>> onscreens;
    Core::Map<RED4ext::CName, Core::Vector<std::string>> subtitles;
};
}
