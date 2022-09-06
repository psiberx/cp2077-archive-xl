#include "Unit.hpp"

namespace
{
constexpr auto FactoryIndexNodeKey = "factories";
}

bool App::FactoryIndexUnit::IsDefined()
{
    return !factories.empty();
}

void App::FactoryIndexUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& factoriesNode = aNode[FactoryIndexNodeKey];

    if (!factoriesNode.IsDefined())
        return;

    bool malformed = false;

    switch (factoriesNode.Type())
    {
    case YAML::NodeType::Sequence:
    {
        for (const auto& pathNode : factoriesNode)
        {
            if (pathNode.IsScalar())
                factories.emplace_back(pathNode.Scalar());
            else
                malformed = true;
        }
        break;
    }
    case YAML::NodeType::Scalar:
    {
        factories.emplace_back(factoriesNode.Scalar());
        break;
    }
    default:
    {
        malformed = true;
    }
    }

    if (malformed)
        issues.emplace_back("Bad format. Expected resource path or list of paths.");
}
