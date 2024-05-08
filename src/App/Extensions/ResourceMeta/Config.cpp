#include "Config.hpp"

bool App::ResourceMetaConfig::IsDefined()
{
    return !aliases.empty() || !fixes.empty();
}

void App::ResourceMetaConfig::LoadYAML(const YAML::Node& aNode)
{
    const auto& rootNode = aNode["resource"];

    if (!rootNode.IsDefined() || !rootNode.IsMap())
        return;

    LoadAliases(rootNode["alias"]);
    LoadFixes(rootNode["fix"]);
}

void App::ResourceMetaConfig::LoadAliases(const YAML::Node& aNode)
{
    if (!aNode.IsDefined() || !aNode.IsMap())
        return;

    for (const auto& entryNode : aNode)
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
                aliases[aliasPath].insert(targetPath);
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
                    aliases[aliasPath].insert(targetPath);
                    paths[targetPath] = targetPathStr;
                }
            }
        }
    }
}

void App::ResourceMetaConfig::LoadFixes(const YAML::Node& aNode)
{
    if (!aNode.IsDefined() || !aNode.IsMap())
        return;

    for (const auto& entryNode : aNode)
    {
        const auto& definitionNode = entryNode.second;

        if (!definitionNode.IsMap())
            continue;

        const auto& namesNode = definitionNode["names"];
        const auto& contextNode = definitionNode["context"];

        if (namesNode.IsDefined() && !namesNode.IsMap())
            continue;

        if (contextNode.IsDefined() && !contextNode.IsMap())
            continue;

        const auto& targetPathStr = entryNode.first.Scalar();
        const auto& targetPath = Red::ResourcePath(targetPathStr.data());

        ResourceFix fixData;

        if (namesNode.IsDefined())
        {
            for (const auto& nameNode : namesNode)
            {
                auto oldName = Red::CNamePool::Add(nameNode.first.Scalar().data());
                auto newName = Red::CNamePool::Add(nameNode.second.Scalar().data());

                fixData.names[oldName] = newName;
            }
        }

        if (contextNode.IsDefined())
        {
            for (const auto& paramNode : contextNode)
            {
                auto paramName = Red::CNamePool::Add(paramNode.first.Scalar().data());
                const auto& paramValue = paramNode.second.Scalar();

                fixData.context[paramName] = paramValue;
            }
        }

        if (!fixData.names.empty() || !fixData.context.empty())
        {
            fixes[targetPath] = std::move(fixData);
            paths[targetPath] = targetPathStr;
        }
    }
}
