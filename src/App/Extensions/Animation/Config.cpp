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

        if (!entityNode.IsDefined())
        {
            malformed = true;
            continue;
        }

        AnimationEntry entry;

        if (entityNode.IsScalar())
        {
            entry.entities.insert(entityNode.Scalar());
        }
        else if (entityNode.IsScalar())
        {
            for (const auto& pathNode : entityNode)
            {
                if (pathNode.IsScalar())
                {
                    entry.entities.insert(pathNode.Scalar());
                }
            }
        }

        if (entry.entities.empty())
        {
            malformed = true;
            continue;
        }

        const auto& animSetNode = entryNode[AnimSetNodeKey];

        if (!animSetNode.IsDefined() || !animSetNode.IsScalar())
        {
            malformed = true;
            continue;
        }

        entry.set = animSetNode.Scalar();

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

        animations.emplace_back(std::move(entry));
    }
}
