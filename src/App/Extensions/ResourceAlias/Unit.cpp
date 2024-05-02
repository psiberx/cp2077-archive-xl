#include "Unit.hpp"

bool App::ResourceAliasUnit::IsDefined()
{
    return !aliases.empty();
}

void App::ResourceAliasUnit::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    const auto& aliasNode = rootNode["alias"];

    if (!aliasNode.IsDefined() || !aliasNode.IsMap())
        return;

    for (const auto& entryNode : aliasNode)
    {
        const auto& aliasPathStr = entryNode.first.Scalar();
        const auto& aliasPath = Red::ResourcePath(aliasPathStr.data());
        const auto& targetsNode = entryNode.second;

        paths[aliasPath] = aliasPathStr;

        if (targetsNode.IsScalar())
        {
            const auto& targetPathStr = targetsNode.Scalar();
            const auto& targetPath = Red::ResourcePath(targetPathStr.data());

            if (targetPath)
            {
                aliases[aliasPath].emplace_back(targetPath);
                paths[targetPath] = targetPathStr;
            }
        }
        else if (targetsNode.IsSequence())
        {
            for (const auto& targetNode : targetsNode)
            {
                const auto& targetPathStr = targetNode.Scalar();
                const auto& targetPath = Red::ResourcePath(targetPathStr.data());

                if (targetPath)
                {
                    aliases[aliasPath].emplace_back(targetPath);
                    paths[targetPath] = targetPathStr;
                }
            }
        }
    }
}
