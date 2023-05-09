#include "Unit.hpp"

namespace
{
constexpr auto AnimationsNodeKey = "animations";
constexpr auto EntityNodeKey = "entity";
constexpr auto AnimSetNodeKey = "set";
constexpr auto VariablesNodeKey = "vars";
}

bool App::AnimationsUnit::IsDefined()
{
    return !animations.empty();
}

void App::AnimationsUnit::LoadYAML(const YAML::Node& aNode)
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
    }

    if (malformed)
    {
        issues.emplace_back("Bad format. Expected list of anim entries.");
    }
}
