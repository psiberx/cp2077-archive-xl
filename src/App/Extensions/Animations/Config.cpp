#include "Config.hpp"
#include "App/Utils/Num.hpp"

namespace
{
constexpr auto AnimationsNodeKey = "animations";
constexpr auto EntityNodeKey = "entity";
constexpr auto AnimSetNodeKey = "set";
constexpr auto VariablesNodeKey = "vars";
constexpr auto PriorityNodeKey = "priority";
constexpr auto ComponentNodeKey = "component";
}

bool App::AnimationsConfig::IsDefined()
{
    return !animations.empty();
}

void App::AnimationsConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& animationsNode = aNode[AnimationsNodeKey];

    if (!animationsNode.IsDefined())
        return;

    if (!animationsNode.IsSequence())
    {
        issues.emplace_back("Bad format. Expected list of anim entries.");
        return;
    }

    bool malformed = false;

    for (const auto& entryNode : animationsNode)
    {
        const auto& entityNode = entryNode[EntityNodeKey];
        const auto& animSetNode = entryNode[AnimSetNodeKey];

        if (!entityNode.IsDefined() || !entityNode.IsScalar() || !animSetNode.IsDefined() || !animSetNode.IsScalar())
        {
            malformed = true;
            continue;
        }

        auto& entry = animations.emplace_back(entityNode.Scalar(), animSetNode.Scalar());

        const auto& variablesNode = entryNode[VariablesNodeKey];
        if (variablesNode.IsDefined() && variablesNode.IsSequence())
        {
            for (const auto& variableNode : variablesNode)
            {
                if (variablesNode.IsScalar())
                {
                    entry.variables.push_back(variableNode.Scalar());
                }
            }
        }

        const auto& priorityNode = entryNode[PriorityNodeKey];
        if (priorityNode.IsDefined() && priorityNode.IsScalar())
        {
            ParseInt(priorityNode.Scalar(), entry.priority);
        }

        const auto& componentNode = entryNode[ComponentNodeKey];
        if (componentNode.IsDefined() && componentNode.IsScalar())
        {
            entry.component = componentNode.Scalar();
        }
    }

    if (malformed)
    {
        issues.emplace_back("Bad format. Expected list of anim entries.");
    }
}
