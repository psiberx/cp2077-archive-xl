#include "Unit.hpp"

bool App::EntityOverrideUnit::IsDefined()
{
    return !overrides.empty();
}

void App::EntityOverrideUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& mergeNode = aNode["merge"];

    if (!mergeNode.IsDefined())
        return;

    bool malformed = false;

    switch (mergeNode.Type())
    {
    case YAML::NodeType::Map:
    {
        for (const auto& entryNode : mergeNode)
        {
            const auto& targetPath = Red::ResourcePath(entryNode.first.Scalar().data());
            const auto& overridesNode = entryNode.second;

            if (overridesNode.IsScalar())
            {
                overrides[targetPath].emplace_back(overridesNode.Scalar());
            }
            else if (overridesNode.IsSequence())
            {
                for (const auto& overrideNode : overridesNode)
                {
                    overrides[targetPath].emplace_back(overrideNode.Scalar());
                }
            }
        }
        break;
    }
    default:
    {
        malformed = true;
    }
    }

    if (malformed)
        issues.emplace_back("Bad format. Expected a map of resource paths.");
}
