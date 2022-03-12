#pragma once

#include "App/Common/Unit.hpp"

namespace App
{
struct FactoryIndexUnit : Unit
{
    using Unit::Unit;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Vector<std::string> factories;
};
}
