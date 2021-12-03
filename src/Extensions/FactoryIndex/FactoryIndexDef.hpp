#pragma once

#include "Definition.hpp"
#include "Reverse/ResourcePath.hpp"

namespace AXL
{
struct FactoryIndexDefinition : public Definition
{
    using ResourceList = std::vector<ResourcePath>;

    FactoryIndexDefinition(const std::string& aName)
        : Definition(aName) {}

    bool IsDefined() override;

    static FactoryIndexDefinition FromConfig(const std::string& aName, const YAML::Node& aNode);

    ResourceList factories;
};
}
