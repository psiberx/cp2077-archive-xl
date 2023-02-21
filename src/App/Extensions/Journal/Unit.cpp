#include "Unit.hpp"

namespace
{
constexpr auto JournalNodeKey = "journal";
}

bool App::JournalUnit::IsDefined()
{
    return !journals.empty();
}

void App::JournalUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode[JournalNodeKey];

    if (!rootNode.IsDefined())
        return;

    bool malformed = false;

    switch (rootNode.Type())
    {
    case YAML::NodeType::Sequence:
    {
        for (const auto& pathNode : rootNode)
        {
            if (pathNode.IsScalar())
                journals.emplace_back(pathNode.Scalar());
            else
                malformed = true;
        }
        break;
    }
    case YAML::NodeType::Scalar:
    {
        journals.emplace_back(aNode.Scalar());
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
