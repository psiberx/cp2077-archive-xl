#include "Config.hpp"

namespace
{
constexpr auto CustomizationNodeKey = "customizations";
constexpr auto MaleNodeKey = "male";
constexpr auto FemaleNodeKey = "female";
}

bool App::CustomizationConfig::IsDefined()
{
    return !maleOptions.empty() || !femaleOptions.empty();
}

void App::CustomizationConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode[CustomizationNodeKey];

    if (!rootNode.IsDefined())
        return;

    bool malformed = false;

    if (!ReadOptions(rootNode[MaleNodeKey], maleOptions))
        malformed = true;

    if (!ReadOptions(rootNode[FemaleNodeKey], femaleOptions))
        malformed = true;

    if (malformed)
        issues.emplace_back("Bad format. Expected resource path or list of paths.");
}

bool App::CustomizationConfig::ReadOptions(const YAML::Node& aNode, Core::Vector<std::string>& aOptions)
{
    bool malformed = false;

    if (aNode.IsDefined())
    {
        switch (aNode.Type())
        {
        case YAML::NodeType::Sequence:
        {
            for (const auto& pathNode : aNode)
            {
                if (pathNode.IsScalar())
                    aOptions.emplace_back(pathNode.Scalar());
                else
                    malformed = true;
            }
            break;
        }
        case YAML::NodeType::Scalar:
        {
            aOptions.emplace_back(aNode.Scalar());
            break;
        }
        default:
        {
            malformed = true;
        }
        }
    }

    return !malformed;
}
