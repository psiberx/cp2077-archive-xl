#include "FactoryIndexDef.hpp"

AXL::FactoryIndexDefinition AXL::FactoryIndexDefinition::FromConfig(const std::string& aName, const YAML::Node& aNode)
{
    FactoryIndexDefinition definition(aName);

    const auto& factoriesNode = aNode["factories"];

    if (factoriesNode.IsDefined())
    {
        if (factoriesNode.Type() == YAML::NodeType::Scalar)
        {
            definition.factories.emplace_back(factoriesNode.as<std::string>().c_str());
        }
        else if (factoriesNode.Type() == YAML::NodeType::Sequence)
        {
            for (const auto& resourceNode : factoriesNode)
            {
                if (resourceNode.Type() == YAML::NodeType::Scalar)
                    definition.factories.emplace_back(resourceNode.as<std::string>().c_str());
            }
        }
    }

    return std::move(definition);
}

bool AXL::FactoryIndexDefinition::IsDefined()
{
    return !factories.empty();
}
