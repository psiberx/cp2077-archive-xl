#include "Config.hpp"

namespace
{
constexpr auto PlayerNodeKey = "player";
constexpr auto BodyTypesNodeKey = "bodyTypes";
}

bool App::PuppetStateConfig::IsDefined()
{
    return !bodyTypes.empty();
}

void App::PuppetStateConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& playerNode = aNode[PlayerNodeKey];

    if (!playerNode.IsDefined())
        return;

    bool malformed = false;

    const auto& bodyTypesNode = playerNode[BodyTypesNodeKey];

    if (bodyTypesNode.IsDefined())
    {
        switch (bodyTypesNode.Type())
        {
        case YAML::NodeType::Sequence:
        {
            for (const auto& itemNode : bodyTypesNode)
            {
                if (itemNode.IsScalar())
                    bodyTypes.push_back(itemNode.Scalar());
                else
                    malformed = true;
            }
            break;
        }
        case YAML::NodeType::Scalar:
        {
            bodyTypes.push_back(bodyTypesNode.Scalar());
            break;
        }
        default:
        {
            malformed = true;
        }
        }
    }

    if (malformed)
        issues.emplace_back("Bad format. Expected body type name or list of names.");
}
